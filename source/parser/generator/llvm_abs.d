module parser.generator.llvm_abs;

import parser.generator.gen;
import llvm;
import std.string;
import std.array;
import std.uni;
import std.ascii;

LLVMValueRef createLocal(GenerationContext ctx, LLVMTypeRef type, LLVMValueRef val, string name) {
		LLVMValueRef local = LLVMBuildAlloca(
			ctx.currbuilder,
			type,
			toStringz(name)
		);

		if(val !is null) {
			LLVMBuildStore(
				ctx.currbuilder,
				val,
				local
			);
		}

		ctx.gstack.addLocal(local,name);
		return local;
}

LLVMValueRef createGlobal(GenerationContext ctx, LLVMTypeRef type, LLVMValueRef val, string name) {
		LLVMValueRef global = LLVMAddGlobal(
			ctx.mod,
			type,
			toStringz(name)
		);
		if(val !is null) LLVMSetInitializer(global,val);
		ctx.gstack.addGlobal(global,name);
		return global;
}

void setGlobal(GenerationContext ctx, LLVMValueRef value, string name) {
		LLVMBuildStore(
			ctx.currbuilder,
			value,
			ctx.gstack[name]
		);
}

void setLocal(GenerationContext ctx, LLVMValueRef value, string name) {
		auto tmp = LLVMBuildAlloca(
			ctx.currbuilder,
			LLVMGetAllocatedType(ctx.gstack[name]),
			toStringz(name~"_setlocal")
		);

		if(value !is null) LLVMBuildStore(
			ctx.currbuilder,
			value,
			tmp
		);

		ctx.gstack.set(tmp,name);
}

LLVMValueRef getValueByPtr(GenerationContext ctx, string name) {
		return LLVMBuildLoad(
			ctx.currbuilder,
			ctx.gstack[name],
			toStringz(name~"_valbyptr")
		);
}

LLVMValueRef getVarPtr(GenerationContext ctx, string name) {
		return ctx.gstack[name];
}

LLVMValueRef operAdd(GenerationContext ctx, LLVMValueRef one, LLVMValueRef two, bool isreal) {
        if(!isreal) return LLVMBuildAdd(
			ctx.currbuilder,
			one,
			two,
			toStringz("operaddi_result")
		);

		return LLVMBuildFAdd(
			ctx.currbuilder,
			one,
			two,
			toStringz("operaddf_result")
		);
}

LLVMValueRef operSub(GenerationContext ctx, LLVMValueRef one, LLVMValueRef two, bool isreal) {
		if(!isreal) return LLVMBuildSub(
			ctx.currbuilder,
			one,
			two,
			toStringz("opersubi_result")
		);

		return LLVMBuildFSub(
			ctx.currbuilder,
			one,
			two,
			toStringz("opersubf_result")
		);
}

LLVMValueRef operMul(GenerationContext ctx, LLVMValueRef one, LLVMValueRef two, bool isreal) {
		if(!isreal) return LLVMBuildMul(
			ctx.currbuilder,
			one,
			two,
			toStringz("opermuli_result")
		);

		return LLVMBuildFSub(
			ctx.currbuilder,
			one,
			two,
			toStringz("opermulf_result")
		);
}

LLVMValueRef operDiv(GenerationContext ctx, LLVMValueRef one, LLVMValueRef two, bool isreal) {
		// TODO: Add a LLVMCast
		if(isreal) return LLVMBuildFDiv(
			ctx.currbuilder,
			one,
			two,
			toStringz("operdivf_result")
		);
        assert(0); // TODO: Integer support
}

LLVMValueRef castNum(GenerationContext ctx, LLVMValueRef tocast, LLVMTypeRef type, bool isreal) {
        if(!isreal) return LLVMBuildZExt(
			ctx.currbuilder,
			tocast,
			type,
			toStringz("castNumI")
		);

		return LLVMBuildSExt(
			ctx.currbuilder,
			tocast,
			type,
			toStringz("castNumF")
		);
}

LLVMValueRef ptrToInt(GenerationContext ctx, LLVMValueRef ptr, LLVMTypeRef type) {
		return LLVMBuildPtrToInt(
			ctx.currbuilder,
			ptr,
			type,
			toStringz("ptrToint")
		);
}

LLVMValueRef intToPtr(GenerationContext ctx, LLVMValueRef num, LLVMTypeRef type) {
		return LLVMBuildIntToPtr(
			ctx.currbuilder,
			num,
			type,
			toStringz("intToptr")
		);
}

LLVMTypeRef getAType(LLVMValueRef v) {
		return LLVMGetAllocatedType(v);
}

LLVMTypeRef getVarType(GenerationContext ctx, string n) {
    if(ctx.gstack.isGlobal(n)) {
        return LLVMGlobalGetValueType(ctx.gstack[n]);
    }
    return getAType(ctx.gstack[n]);
}