# Commands

## Inside-The-Loop
**break** - Stop the current loop.
**continue** - Go to the next iteration of the loop.

Example:
```d
int i = 0;
while(i<100) {
    if(i == 40) continue;
    else if(i == 67) break;
    i += 1;
}
```

## Outside-Of-Functions

**import** - Import a file.
To import files near the compiler (for example, std), use the prefix and postfix < and >.
Otherwise, put the file name in double quotes.
You don't need to specify the file extension.

Example:
```d
import <std/io> <std/locale>
import "specifications"
```

## Inside-Of-Functions

**while(cond) [body/{body}]** - A block of code executed every time the conditions are true.

Example:
```d
int i = 0;
while(i<10) {i += 1;}
```

**[if/else](cond) [body/{body}]** - A block of code executed if the conditions are true.

Example:
```d
if(A == B) A = C;
else {
    A = B;
}
```

**for(var; cond; expr) [body/{body}]** - The for loop is like in C, creating a local variable and executing the expression after the block and the block itself if the condition is met.

Example:
```d
for(int i=0; i<100; i+=1) {
    // ...
}
```

**switch(expr) {case(expr) {} default {}}** - It works like a switch in C, except for one thing - break and continue are prohibited in switch.

Example:
```d
int n;
std::input(&n);

switch(n) {
    case(10) {
        std::println("10");
    }
    case(20) {
        std::println("20");
    }
    default {
        std::println("Other");
    }
}
```

**cast(type)expr** - Transformation of an expression from one type to another.

Example:

```d
float a = 0;
int b = cast(float)a;
```

## Built-In-Functions

**@sizeOf(type)** - Get the size of the type.

Example:

```d
int size = @sizeOf(int); // 4
```

**@baseType(type)** - Get the base type from the reduced type. If the type is not a pointer or an array, it is returned unchanged.

Example:

```d
import <std/io>

void main {
    std::println(@sizeOf(@baseType(int*))); // 4
}
```

**@tIsEquals(type1, type2), @tIsNequals(type1, type2)** - Compare the two types with each other.

Example:

```d
@if(@tIsEquals(int,char)) {
    int a = 0;
};
@if(@tIsNequals(int,char)) {
    int a = 5;
};
```

**@isNumeric, @tIsPointer, @tIsArray(type)** - Check whether the type is numerical, pointer or array

Example:

```d
@if(@isNumeric(int)) {
    // ...
};
@if(tIsPointer(int*)) {
    // ...
};
@if(tIsArray(int[1])) {
    // ...
};
```

**@contains(str1, str2)** - Check if str2 is in str1.

Example:

```d
// From std/prelude.rave
@if(@aliasExists(__RAVE_IMPORTED_FROM)) {
    @if(@contains(__RAVE_IMPORTED_FROM, "std/sysc.rave") == false) {
        import <std/sysc>  
    };
};
@if(!@aliasExists(__RAVE_IMPORTED_FROM)) {
    import <std/sysc>
};
```

**@compileAndLink(strings)** - Compile and add files to the linker without importing them.

Example:

```d
// From std/prelude.rave
@compileAndLink("<std/io>");
```

**@typeToString(type)** - Convert the type to a string.

Example:

```d
const(char)* _int = @typeToString(int);
@if(_int == "int") {

};
```

**@aliasExists(name)** - Check if there is an alias with this name.

**@error(strings/aliases), @echo(strings/aliases), @warning(strings/aliases)** - Output strings and/or aliases.

With @warning, only the color changes, but with @error, compilation ends.

Example:

```d
@echo("Hello, ","world!");
@warning("Warning.");
@error("Goodbye!");
```

**@setRuntimeChecks(true/false)** - Enable/disable runtime checks.

**@callWithArgs(args, functionName)** - Call a function with its own arguments and arguments when called.

Example:

```d
void two(int one, int _two) => one + _two;

(ctargs) void one {
    @callWithArgs(2, two);
}
```

### Compile-time arguments

**@getCurrArg(type)** - Get the value of the current argument, leading to the required type.

Example:

```d
(ctargs) int sumOfTwo {
    int one = @getCurrArg(0, int);
    int two = @getArg(1, int);
} => one + two;
```

**@skipArg()** - Skip the current argument.

Example:

```d
(ctargs) int foo {
    @skipArg(); // skip the first argument
    int second = @getCurrArg(int); // getting the second argument as integer
} => second / 2;
```

**@getCurrArgType()** - Get the type of the current argument.

Example:

```d
(ctargs) int bow {
    @if(@tIsEquals(@getCurrArgType(), int)) {
        @echo("Integer");
    };
}
```

**@foreachArgs() {block};** - Generate code for each argument at compile-time.

Example:

```d
(ctargs) int sum {
    int result = 0;

    @foreachArgs() {
        result += @getCurrArg(int);
    };
} => result;
```

**@getArg(n, type), @getArgType(n)** - Functions similar to the top two, except for the need to add an argument number.

Example:

```d
(ctargs) int sum2 {
    @if(@tNequals(@getArgType(0), @getArgType(1))) {
        @error("Different types!");
    };
} => @getArg(0, int) + @getArg(1, int);
```