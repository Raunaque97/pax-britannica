#include <libgen.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

//// preloaders ///////////////////////////////////////////////////////////////
// to statically link in binary lua modules:
//
// - use the right linker options (or just include the .a in the link line)
// - add them to init_preloaders right below
//
// You don't need to manually declare them or include anything, the
// REGISTER_LOADER macro handles that.

#define REGISTER_LOADER(module_name, loader) \
    int loader(lua_State *L); \
    lua_pushcfunction(L, loader); \
    lua_setfield(L, -2, module_name)

void init_preloaders(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    // add your custom loaders here, they look like this:
    REGISTER_LOADER("collision.native", luaopen_collision_native);
    REGISTER_LOADER("gl", luaopen_gl);
    REGISTER_LOADER("glfw", luaopen_glfw);
    REGISTER_LOADER("glu", luaopen_glu);
    REGISTER_LOADER("memarray", luaopen_memarray);
    REGISTER_LOADER("mixer", luaopen_mixer);
    REGISTER_LOADER("stb_image", luaopen_stb_image);

    // each night I pray that I might one day happen upon a less disgusting way
    // of doing this
    #ifdef EXTRA_LOADERS
    #include EXTRA_LOADERS
    #endif
    
    lua_pop(L, 2);
}

//// working directory handling ///////////////////////////////////////////////

#ifdef DOKIDOKI_LINUX

int get_exe_path(char *buffer, int size)
{
    int ret = readlink("/proc/self/exe", buffer, size);
    if(0 >= ret && ret < size)
        buffer[ret] = 0;
    return ret;
}

int switch_to_game_directory()
{
    printf("switching\n");
    int size = 256;
    char *buffer = NULL;
    while(!buffer)
    {
        buffer = (char *)malloc(size);
        int ret = get_exe_path(buffer, size);

        // error
        if(ret <= 0)
        {
            free(buffer);
            buffer = NULL;
            return 0;
        }
        // buffer too small
        else if(ret == size)
        {
            free(buffer);
            buffer = NULL;
            size *= 2;
        }
    }

    char *dir = dirname(buffer);
    printf("switching to %s\n", dir);

    int ret = chdir(dir);
    printf("%d\n", ret);
    free(buffer);
    return ret;
}

#else

int switch_to_game_directory()
{
    printf("automatic game path detection not supported on this platform\n");
    return 0;
}

#endif

//// main program /////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
    switch_to_game_directory();

    // load lua and libraries
    lua_State *L = lua_open();
    luaL_openlibs(L);
    init_preloaders(L);
    
    // load arguments (and pre-arguments) into a global 'arg' table
    lua_newtable(L);
    int i;
    for(i = 0; i < argc; i++)
    {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "arg");

    // open app, with error reporting
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");

    int error = luaL_dofile(L, "init.lua");

    if(!error)
    {
        // load command-line arguments as function arguments
        lua_checkstack(L, argc - 1);
        for(i = 1; i <= argc - 1; i++)
            lua_pushstring(L, argv[i]);
        error = lua_pcall(L, argc - 1, 0, -2);  // run the result
    }

    if(error)
        fprintf(stderr, "%s\n", lua_tostring(L, -1));

    lua_close(L);
    return error;
}

