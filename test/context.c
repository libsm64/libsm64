#include "context.h"

#include <string.h>

static SDL_Window *s_sdlWindow;
static SDL_GLContext s_sdlGlContext;
static SDL_GameController *s_controller;
int WINDOW_WIDTH;
int WINDOW_HEIGHT;

void context_init( const char *title, int width, int height, int major, int minor )
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 ) goto err;

    int profile = (major < 3) ? SDL_GL_CONTEXT_PROFILE_COMPATIBILITY : SDL_GL_CONTEXT_PROFILE_CORE;

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, major );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, minor );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, profile );

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    s_sdlWindow = SDL_CreateWindow( 
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if( !s_sdlWindow ) goto err;

    s_sdlGlContext = SDL_GL_CreateContext( s_sdlWindow );
    SDL_GL_SetSwapInterval( 1 );

    if( !s_sdlGlContext ) goto err;

    #ifndef __APPLE__
        glewExperimental = GL_TRUE;
        const GLenum glewInitResult = glewInit();
        if( glewInitResult != GLEW_OK ) goto err;
    #endif

    s_controller = NULL;

    for( int i = 0; i < SDL_NumJoysticks(); ++i ) 
    {
        if( ! SDL_IsGameController( i ) ) continue;
        s_controller = SDL_GameControllerOpen( i );
        printf( "Using game controller: %s", SDL_GameControllerName( s_controller ) );
        if( s_controller ) break;
    }

    return;

err:
    SDL_GL_DeleteContext( s_sdlGlContext );
    SDL_DestroyWindow( s_sdlWindow );
    SDL_Quit();
}

SDL_GameController *context_get_controller( void )
{
    return s_controller;
}

bool context_flip_frame_poll_events( void )
{
    bool still_running = true;

    SDL_GL_SwapWindow( s_sdlWindow );

    SDL_Event event;

    while( SDL_PollEvent( &event ) )
    {
        switch( event.type )
        {
        case SDL_QUIT:
            still_running = false;
            break;

        case SDL_KEYDOWN:
            if( event.key.keysym.sym == SDLK_ESCAPE )
                still_running = false;

        case SDL_WINDOWEVENT:
            if( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
            {
                WINDOW_WIDTH = event.window.data1;
                WINDOW_HEIGHT = event.window.data2;
                glViewport( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT );
            }
            break;
        }
    }

    return still_running;
}

void context_terminate( void )
{
    if( !s_sdlWindow )
        return;

    if( s_controller )
        SDL_GameControllerClose( s_controller );

    SDL_GL_DeleteContext( s_sdlGlContext );
    SDL_DestroyWindow( s_sdlWindow );
    SDL_Quit();

    s_sdlWindow = NULL;
}
