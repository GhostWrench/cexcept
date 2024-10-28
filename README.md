# cexcept : C Exceptions

This is a minimal exception handling library for C. It provides a few macros for throwing and checking for errors as well as a simple method for freeing any allocated memory at the time of an exception.

## Building

To build in a Linux environment you will need a system with a C compiler and make. cd into the root directory with the Makefile and run:

```sh
$ make all
```

This will build with the default options:

* `BUILD_CFG=release` 
* `SHARED_LIB=1`

They can be changed by invoking make and setting the option like so:

```sh
$ make BUILD_CFG=debug all
```

## Using cexcept

### Basic usage

A functions which make use of the macros defined in the library must have a return type of `cexcept`. You can exit a function immediately by throwing an exception.

```c
cexcept my_func() {
    ...
    if (bad_thing_happened) {
        CEXCEPT_THROW("Some %s happened!\n", "thing");
    }
    ...
    return CEXCEPT_OK;
}
```

You can handle the exception in the calling function with basic if statements

```c
void my_calling_func() {
    if (!my_func()) {
        handle_the_exception();
    }
}
```

Or if your calling function has the cexcept return type you can pass the exception up the call stack

```c
cexcept my_calling_func2() {
    CEXCEPT_CHECK(my_func());
    return CEXCEPT_OK;
}
```

### Handling heap memory

Many times, memory cleanup needs to be done before exiting a function. cexcept provides a basic datatype to help cleanup memory `cexcept_free_list` that you can add pointers and free functions to that can be automatically free'd with the `CEXCEPT_THROW_F` and `CEXCEPT_CHECK_F` macros.

```c
cexcept complex_function() {
    cexcept_free_list *free_list = cexcept_free_list_new();
    int *test = malloc(100*sizeof(int));
    cexcept_free_list_add(free_list, test, free);
    ...
    if (bad_thing_happened) {
        CEXCEPT_THROW_F(free_list, "A bad thing happened!\n");
    }
    ...
    CEXCEPT_CHECK_F(free_list, dodgy_function());
    ...
    cexcept_free(free_list);
    return CEXCEPT_OK;
}
```

You may have noticed that the cexcept_free_list_add has a `cexcept` return type. And you may be tempted to use `CEXCEPT_CHECK_F` with it. This is generally a bad idea because it will not free the memory that you are attempting to add to it if it fails.

```c
cexcept complex_function_2() {
    cexcept_free_list *free_list = cexcept_free_list_new();
    int *test = malloc(1000*sizeof(int)); // <-- This memory will leak if the add fails
    CEXCEPT_CHECK_F(free_list, cexcept_free_list_add(free_list, test, free));
    ...
}
```

Instead, use the `CEXCEPT_CHECK_ADD` macro

```c
cexcept complex_function_2() {
    cexcept_free_list *free_list = cexcept_free_list_new();
    int *test = malloc(1000*sizeof(int)); // <-- This will be free'd before return if the add fails
    CEXCEPT_CHECK_ADD(free_list, test, free);
    ...
}
```

If you have a more complex datatype that has a special free function with the type `void (*)(<any_type>*)` You can pass a pointer to that function to the free list instead of `NULL` or `free`. It works with (FILE*)s and fclose, and probably other common system resource calls.

```c
typedef struct {
    int *data1;
    int *data2;
} complex_data;

complex_data* complex_data_init() {
    complex_data *cd = malloc(sizeof(complex_data));
    cd->data1 = malloc(100*sizeof(int));
    cd->data2 = malloc(100*sizeof(int));
    return cd;
}

void complex_data_free(complex_data *cd) {
    if (cd) {
        if (cd->data1) free(cd->data1);
        if (cd->data2) free(cd->data2);
        free(cd);
    }
}

int main() {
    cexcept_free_list *free_list = cexcept_free_list_new();
    ...
    complex_data *my_data = complex_data_init();
    cexcept_free_list_add(free_list, my_data, complex_data_free);
    ...
    FILE *my_file = fopen("./file.txt", "r");
    cexcept_free_list_add(free_list, my_file, fclose);
    ...
    // Free my_file resource early
    cexcept_free_list_remove(free_list, my_file, true);
    ...
    cexcept_free(free_list);
    return CEXCEPT_OK;
}
```

## FLCs: Your Frequently Lodged Complaints, Answered!

NOTE: The following is meant to be (mostly) tongue-in-cheek, please don't send me angry emails!

C: This library forces the use of function output parameters for errors? Gross! that is not very elegant. Can't you come up with anything more clever?

A: No, use the function call parameters for output like a real developer. You do know how to use pointers right? It is clear from the design of C that function outputs were only meant for exception handling.

C: Why is there no try, catch, finally? Everyone knows that real exceptions have a try catch and finally and exception types!

A: C has 'if' statements, use those instead. What? Are you programming a moonlander or something? Yeesh! If you really can't live without them you should have a look at [exceptions4c](https://github.com/guillermocalvo/exceptions4c).

C: Your implementation of `cexcept_free_table` is really slow! You should use a  hash map for O(1) inserts and removes!

A: Exactly how many resources are you using at one time that you want an effing hash map? If it is more than 10 maybe you should re-consider your career and all your life choices that lead you to this situation. Plus, I read somewhere on the internet that casting pointers to ints so they can be hashed is UB, and I got scared.

C: Why won't you accept my pull request?

A: Just fork the repo or better yet, copy-paste the .c and .h files into your project! It's like ~250 lines of code that I wrote in a weekend. If your mega-corp is dumb enough to use this repo as a dependency then I have no words. I am not doing free labor so your billionaire overlords can have another yacht.

C: You really should be using `whiz-bang-fancy-pants-tool` why aren't you using `whiz-bang-fancy-pants-tool`?

A: You must be lost, this is a library written in C, not C++, Java or JavaScript. Use of any tools more complicated than what what written by GNU in the 1980s is expressly verboten by our lord and savior Linus Torvalds. (except for git)

C: You can't just do stuff like that with (void *) you maniac! Stop it right now and end this monstrosity of a library before our Jr. devs put it in production and I have to debug it!

A: Lol, (void *) go brrrrrr!
