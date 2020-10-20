#include "context.h"

#include <string.h>

static SDL_Window *s_sdlWindow;
static SDL_GLContext s_sdlGlContext;
static SDL_GameController *s_controller;
static int s_windowWidth;
static int s_windowHeight;

void context_init( const char *title, int width, int height )
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 ) goto err;

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetSwapInterval( 1 );

    s_windowWidth = width;
    s_windowHeight = height;

    s_sdlWindow = SDL_CreateWindow( 
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if( !s_sdlWindow ) goto err;

    s_sdlGlContext = SDL_GL_CreateContext( s_sdlWindow );

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
                s_windowWidth = event.window.data1;
                s_windowHeight = event.window.data2;
                glViewport( 0, 0, s_windowWidth, s_windowHeight );
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