/*
This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../include/parser/ast.hpp"
#include "../include/parser/Types.hpp"
#include "../include/utils.hpp"
#include "../include/parser/nodes/NodeStruct.hpp"
#include "../include/parser/nodes/NodeVar.hpp"
#include "../include/parser/nodes/NodeInt.hpp"
#include "../include/parser/nodes/NodeString.hpp"
#include "../include/parser/nodes/NodeFloat.hpp"
#include <iostream>

// Type

// TypeBasic
TypeBasic::TypeBasic(char ty) {
    this->type = ty;
}

Type* TypeBasic::copy() {
    return new TypeBasic(this->type);
}

int TypeBasic::getSize() {
    switch(this->type) {
        case BasicType::Bool: return 1;
        case BasicType::Char: case BasicType::Uchar: return 8;
        case BasicType::Short: case BasicType::Ushort: case BasicType::Half:case BasicType::Bhalf: return 16;
        case BasicType::Int: case BasicType::Uint: case BasicType::Float: return 32;
        case BasicType::Long: case BasicType::Ulong: case BasicType::Double: return 64;
        case BasicType::Cent: case BasicType::Ucent: return 128;
        default: return 0;
    }
}

bool TypeBasic::isFloat() {return (this->type == BasicType::Float) || (this->type == BasicType::Double || this->type == BasicType::Half || this->type == BasicType::Bhalf);}

std::string TypeBasic::toString() {
    switch(this->type) {
        case BasicType::Bool: return "bool";
        case BasicType::Char: return "char"; case BasicType::Uchar: return "uchar";
        case BasicType::Short: return "short"; case BasicType::Ushort: return "ushort";
        case BasicType::Int: return "int"; case BasicType::Uint: return "uint";
        case BasicType::Float: return "float";
        case BasicType::Long: return "long"; case BasicType::Ulong: return "ulong";
        case BasicType::Double: return "double";
        case BasicType::Cent: return "cent"; case BasicType::Ucent: return "cent";
        case BasicType::Half: return "half"; case BasicType::Bhalf: return "bhalf";
        default: return "BasicUnknown";
    }
}

Type* TypeBasic::check(Type* parent) {return nullptr;}
Type* TypeBasic::getElType() {return this;}

std::map<char, TypeBasic*> basicTypes;

// TypePointer
TypePointer::TypePointer(Type* instance) {
    this->instance = instance;
}

Type* TypePointer::copy() {
    return new TypePointer(instance->copy());
}

Type* TypePointer::check(Type* parent) {
    if(!instanceof<TypeBasic>(this->instance)) this->instance->check(this);
    return nullptr; 
}

int TypePointer::getSize() {return 8;}
std::string TypePointer::toString() {return instance->toString() + "*";}
Type* TypePointer::getElType() {
    while(instanceof<TypeConst>(instance)) instance = instance->getElType();
    return instanceof<TypeVoid>(instance) ? new TypeBasic(BasicType::Char) : instance;
}

// TypeArray
TypeArray::TypeArray(Node* count, Type* element) {
    this->count = count;
    this->element = element;
}

Type* TypeArray::copy() {
    return new TypeArray(count, element->copy());
}

Type* TypeArray::check(Type* parent) {
    if(!instanceof<TypeBasic>(this->element)) this->element->check(this);
    return nullptr;
}

int TypeArray::getSize() {return ((NodeInt*)this->count->comptime())->value.to_int() * this->element->getSize();}
std::string TypeArray::toString() {return this->element->toString() + "[" + std::to_string(((NodeInt*)this->count->comptime())->value.to_int()) + "]";}
Type* TypeArray::getElType() {return element;}

// TypeAlias
TypeAlias::TypeAlias() {}
Type* TypeAlias::copy() {return new TypeAlias();}
std::string TypeAlias::toString() {return "alias";}
Type* TypeAlias::check(Type* parent) {return nullptr;}
int TypeAlias::getSize() {return 0;}
Type* TypeAlias::getElType() {return this;}

// TypeVoid
TypeVoid::TypeVoid() {}
Type* TypeVoid::check(Type* parent) {return nullptr;}
Type* TypeVoid::copy() {return new TypeVoid();}
int TypeVoid::getSize() {return 0;}
std::string TypeVoid::toString() {return "void";}
Type* TypeVoid::getElType() {return this;}

// TypeConst
TypeConst::TypeConst(Type* instance) {this->instance = instance;}
Type* TypeConst::copy() {return new TypeConst(this->instance->copy());}
Type* TypeConst::check(Type* parent) {this->instance->check(nullptr); return nullptr;}
int TypeConst::getSize() {return this->instance->getSize();}
std::string TypeConst::toString() {return this->instance->toString();}
Type* TypeConst::getElType() {return this->instance->getElType();}

// TypeStruct
TypeStruct::TypeStruct(std::string name) {
    this->name = name;
}

TypeStruct::TypeStruct(std::string name, std::vector<Type*> types) {
    this->name = name;
    this->types = types;
}

Type* TypeStruct::copy() {
    std::vector<Type*> typesCopy;
    for(int i=0; i<this->types.size(); i++) typesCopy.push_back(this->types[i]->copy());
    return new TypeStruct(this->name, typesCopy);
}

void TypeStruct::updateByTypes() {
    if(this->name.find('<') != std::string::npos) {
        this->name = this->name.substr(0, this->name.find('<')) + "<";
        for(int i=0; i<this->types.size(); i++) this->name += this->types[i]->toString() + ",";
        this->name = this->name.substr(0, this->name.size()-1) + ">";
    }
}

int TypeStruct::getSize() {
    Type* t = this;

    while(generator->toReplace.find(t->toString()) != generator->toReplace.end()) t = generator->toReplace[t->toString()];
    while(AST::aliasTypes.find(t->toString()) != AST::aliasTypes.end()) t = AST::aliasTypes[t->toString()];

    if(instanceof<TypeStruct>(t)) {
        TypeStruct* ts = (TypeStruct*)t;
        if(AST::structTable.find(ts->name) == AST::structTable.end()) generator->error("undefined structure '" + ts->name + "'!", -1);
        if(!ts->types.empty()) {
            AST::structTable[ts->name]->genWithTemplate(ts->name.substr(ts->name.find('<')), ts->types);
        }
        int size = 0;
        for(int i=0; i<AST::structTable[ts->name]->elements.size(); i++) {
            if(AST::structTable[ts->name]->elements[i] != nullptr && instanceof<NodeVar>(AST::structTable[ts->name]->elements[i])) {
                NodeVar* nvar = (NodeVar*)AST::structTable[ts->name]->elements[i];
                size += nvar->type->getSize();
            }
        }
        return size;
    }
    return t->getSize();
}

bool TypeStruct::isSimple() {
    Type* t = this;

    while(generator->toReplace.find(t->toString()) != generator->toReplace.end()) t = generator->toReplace[t->toString()];
    while(AST::aliasTypes.find(t->toString()) != AST::aliasTypes.end()) t = AST::aliasTypes[t->toString()];

    if(instanceof<TypeStruct>(t)) {
        TypeStruct* ts = (TypeStruct*)t;
        if(AST::structTable.find(ts->name) == AST::structTable.end()) generator->error("undefined structure '" + ts->name + "'!", -1);
        if(!ts->types.empty()) {
            AST::structTable[ts->name]->genWithTemplate(ts->name.substr(ts->name.find('<')), ts->types);
        }

        for(int i=0; i<AST::structTable[ts->name]->elements.size(); i++) {
            if(AST::structTable[ts->name]->elements[i] != nullptr && instanceof<NodeVar>(AST::structTable[ts->name]->elements[i])) {
                NodeVar* nvar = (NodeVar*)AST::structTable[ts->name]->elements[i];
                if(!instanceof<TypeBasic>(nvar->type)) return false;
            }
        }

        return true;
    }
    return true;
}

int TypeStruct::getElCount() {
    Type* t = this;

    while(generator->toReplace.find(t->toString()) != generator->toReplace.end()) t = generator->toReplace[t->toString()];
    while(AST::aliasTypes.find(t->toString()) != AST::aliasTypes.end()) t = AST::aliasTypes[t->toString()];

    if(instanceof<TypeStruct>(t)) {
        TypeStruct* ts = (TypeStruct*)t;
        if(AST::structTable.find(ts->name) == AST::structTable.end()) generator->error("undefined structure '" + ts->name + "'!", -1);
        if(!ts->types.empty()) {
            AST::structTable[ts->name]->genWithTemplate(ts->name.substr(ts->name.find('<')), ts->types);
        }

        int size = 0;

        for(int i=0; i<AST::structTable[ts->name]->elements.size(); i++) {
            if(AST::structTable[ts->name]->elements[i] != nullptr && instanceof<NodeVar>(AST::structTable[ts->name]->elements[i])) size += 1;
        }

        return size;
    }

    return 1;
}

Type* TypeStruct::check(Type* parent) {
    if(AST::aliasTypes.find(this->name) != AST::aliasTypes.end()) {
        Type* _t = AST::aliasTypes[name];
        while(AST::aliasTypes.find(_t->toString()) != AST::aliasTypes.end()) {
            _t = AST::aliasTypes[_t->toString()];
        }
            
        if(parent == nullptr) return _t;

        if(instanceof<TypePointer>(parent)) {
            TypePointer* tp = (TypePointer*) parent;
            tp->instance = _t;
        }
        else if(instanceof<TypeArray>(parent)) {
            TypeArray* ta = (TypeArray*) parent;
            ta->element = _t;
        }
        else if(instanceof<TypeConst>(parent)) {
            TypeConst* tc = (TypeConst*) parent;
            tc->instance = _t;
        }
    }

    return nullptr;
}

std::string TypeStruct::toString() {return this->name;}
Type* TypeStruct::getElType() {return this;}

// TypeTemplateMember
TypeTemplateMember::TypeTemplateMember(Type* type, Node* value) {
    this->type = type;
    this->value = value;
}

Type* TypeTemplateMember::copy() {return new TypeTemplateMember(this->type->copy(), this->value);}
Type* TypeTemplateMember::check(Type* parent) {return nullptr;}
std::string TypeTemplateMember::toString() {
    if(instanceof<NodeInt>(value)) return "@" + type->toString() + ((NodeInt*)value)->value.to_string();
    else if(instanceof<NodeFloat>(value)) return "@" + type->toString() + std::to_string(((NodeFloat*)value)->value);
    else if(instanceof<NodeString>(value)) return "@" + type->toString() + "\"" + ((NodeString*)value)->value + "\"";
    return "@" + type->toString();
}
int TypeTemplateMember::getSize() {return this->type->getSize();}
Type* TypeTemplateMember::getElType() {return this->type->getElType();}

// TypeTemplateMemberDefinition
TypeTemplateMemberDefinition::TypeTemplateMemberDefinition(Type* type, std::string name) {
    this->type = type;
    this->name = name;
}

Type* TypeTemplateMemberDefinition::copy() {return new TypeTemplateMemberDefinition(this->type->copy(), this->name);}
Type* TypeTemplateMemberDefinition::check(Type* parent) {return nullptr;}
std::string TypeTemplateMemberDefinition::toString() {return this->name;}
int TypeTemplateMemberDefinition::getSize() {return this->type->getSize();}
Type* TypeTemplateMemberDefinition::getElType() {return this->type->getElType();}

// TypeFuncArg
TypeFuncArg::TypeFuncArg(Type* type, std::string name) {
    this->type = type;
    this->name = name;
}

Type* TypeFuncArg::copy() {return new TypeFuncArg(this->type->copy(), this->name);}
Type* TypeFuncArg::check(Type* parent) {return nullptr;}
std::string TypeFuncArg::toString() {return this->name;}
int TypeFuncArg::getSize() {return this->type->getSize();}
Type* TypeFuncArg::getElType() {return this->type->getElType();}

// TypeFunc
TypeFunc::TypeFunc(Type* main, std::vector<TypeFuncArg*> args, bool isVarArg) {
    this->main = main;
    this->args = args;
    this->isVarArg = isVarArg;
}

int TypeFunc::getSize() {return 8;}

Type* TypeFunc::copy() {
    std::vector<TypeFuncArg*> _copied;

    for(int i=0; i<this->args.size(); i++) _copied.push_back((TypeFuncArg*)this->args[i]->copy());
    return new TypeFunc(this->main->copy(), _copied, isVarArg);
}

std::string TypeFunc::toString() {return "NotImplemented";}
Type* TypeFunc::check(Type* parent) {return nullptr;}
Type* TypeFunc::getElType() {return this;}

// TypeBuiltin
TypeBuiltin::TypeBuiltin(std::string name, std::vector<Node*> args, NodeBlock* block) {
    this->name = name;
    this->args = args;
    this->block = block;
}

int TypeBuiltin::getSize() {return 0;}
Type* TypeBuiltin::copy() {return new TypeBuiltin(this->name, this->args, (NodeBlock*)(this->block->copy()));}
std::string TypeBuiltin::toString() {return this->name;}
Type* TypeBuiltin::check(Type* parent) {return nullptr;}
Type* TypeBuiltin::getElType() {return this;}

// TypeCall
TypeCall::TypeCall(std::string name, std::vector<Node*> args) {
    this->name = name;
    this->args = args;
}

std::string TypeCall::toString() {return "FuncCall";}
Type* TypeCall::copy() {return new TypeCall(this->name, this->args);}
int TypeCall::getSize() {return 0;}
Type* TypeCall::check(Type* parent) {return nullptr;}
Type* TypeCall::getElType() {return this;}

// TypeAuto
TypeAuto::TypeAuto() {}
Type* TypeAuto::copy() {return new TypeAuto();}
int TypeAuto::getSize() {return 0;}
Type* TypeAuto::check(Type* parent) {return nullptr;}
std::string TypeAuto::toString() {return "";}
Type* TypeAuto::getElType() {return this;}

// TypeLLVM
TypeLLVM::TypeLLVM(LLVMTypeRef tr) {this->tr = tr;}
Type* TypeLLVM::copy() {return new TypeLLVM(this->tr);}
int TypeLLVM::getSize() {return 0;}
Type* TypeLLVM::check(Type* parent) {return nullptr;}
std::string TypeLLVM::toString() {return "";}
Type* TypeLLVM::getElType() {return this;}

// TypeVector
TypeVector::TypeVector(Type* mainType, int count) {this->mainType = mainType; this->count = count;}
Type* TypeVector::copy() {return new TypeVector(mainType, count);}
int TypeVector::getSize() {return mainType->getSize() * count;}
Type* TypeVector::check(Type* parent) {return nullptr;}
std::string TypeVector::toString() {return "<" + mainType->toString() + " x " + std::to_string(count) + ">";}
Type* TypeVector::getElType() {return mainType;}

// TypeDivided
TypeDivided::TypeDivided(Type* mainType, std::vector<Type*> divided) {this->mainType = mainType; this->divided = divided;}
Type* TypeDivided::copy() {return new TypeDivided(mainType, divided);}

int TypeDivided::getSize() {
    int sum = 0;
    for(int i=0; i<divided.size(); i++) sum += divided[i]->getSize();
    return sum;
}

Type* TypeDivided::check(Type* parent) {return nullptr;}

std::string TypeDivided::toString() {
    return this->mainType->toString() + " {" + std::to_string(this->divided.size()) + " x " + this->divided[0]->toString() + "}";
}

Type* TypeDivided::getElType() {return this;}

TypeVoid* typeVoid;

Type* getType(std::string id) {
    static const std::map<std::string, Type*> types = {
        {"bool", basicTypes[BasicType::Bool]},
        {"char", basicTypes[BasicType::Char]},
        {"uchar", basicTypes[BasicType::Uchar]},
        {"short", basicTypes[BasicType::Short]},
        {"ushort", basicTypes[BasicType::Ushort]},
        {"int", basicTypes[BasicType::Int]},
        {"uint", basicTypes[BasicType::Uint]},
        {"long", basicTypes[BasicType::Long]},
        {"ulong", basicTypes[BasicType::Ulong]},
        {"cent", basicTypes[BasicType::Cent]},
        {"ucent", basicTypes[BasicType::Ucent]},
        {"half", basicTypes[BasicType::Half]},
        {"bhalf", basicTypes[BasicType::Bhalf]},
        {"float", basicTypes[BasicType::Float]},
        {"double", basicTypes[BasicType::Double]},
        {"void", typeVoid},
        {"alias", new TypeAlias()},
        {"int4", new TypeVector(new TypeBasic(BasicType::Int), 4)},
        {"int8", new TypeVector(new TypeBasic(BasicType::Int), 8)},
        {"float4", new TypeVector(new TypeBasic(BasicType::Float), 4)},
        {"float2", new TypeVector(new TypeBasic(BasicType::Float), 2)},
        {"float8", new TypeVector(new TypeBasic(BasicType::Float), 8)},
        {"short8", new TypeVector(new TypeBasic(BasicType::Short), 8)},
    };

    auto it = types.find(id);
    if(it != types.end()) return it->second;
    else return new TypeStruct(id);
}
