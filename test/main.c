#define _CRT_SECURE_NO_WARNINGS 1 // for fopen

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "../src/libsm64.h"

#include "cglm.h"
#include "ns_clock.h"
#include "level.h"
#include "context.h"

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 800;

typedef struct CollisionMesh
{
    size_t num_vertices;
    float *position;
    float *normal;
    uint16_t *index;

    GLuint vao;
    GLuint position_buffer;
    GLuint normal_buffer;
}
CollisionMesh;

typedef struct MarioMesh
{
    size_t num_vertices;
    uint16_t *index;

    GLuint vao;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint color_buffer;
    GLuint uv_buffer;
}
MarioMesh;

typedef struct RenderState
{
    CollisionMesh collision;
    MarioMesh mario;
    GLuint world_shader;
    GLuint mario_shader;
    GLuint mario_texture;
}
RenderState;

static const char *MARIO_SHADER =
"\n uniform mat4 view;"
"\n uniform mat4 projection;"
"\n uniform sampler2D marioTex;"
"\n "
"\n v2f vec3 v_color;"
"\n v2f vec3 v_normal;"
"\n v2f vec3 v_light;"
"\n v2f vec2 v_uv;"
"\n "
"\n #ifdef VERTEX"
"\n "
"\n     layout(location = 0) in vec3 position;"
"\n     layout(location = 1) in vec3 normal;"
"\n     layout(location = 2) in vec3 color;"
"\n     layout(location = 3) in vec2 uv;"
"\n "
"\n     void main()"
"\n     {"
"\n         v_color = color;"
"\n         v_normal = normal;"
"\n         v_light = transpose( mat3( view )) * normalize( vec3( 1 ));"
"\n         v_uv = uv;"
"\n "
"\n         gl_Position = projection * view * vec4( position, 1. );"
"\n     }"
"\n "
"\n #endif"
"\n #ifdef FRAGMENT"
"\n "
"\n     out vec4 color;"
"\n "
"\n     void main() "
"\n     {"
"\n         float light = .5 + .5 * clamp( dot( v_normal, v_light ), 0., 1. );"
"\n         vec4 texColor = texture2D( marioTex, v_uv );"
"\n         vec3 mainColor = mix( v_color, texColor.rgb, texColor.a ); // v_uv.x >= 0. ? texColor.a : 0. );"
"\n         color = vec4( mainColor * light, 1 );"
"\n     }"
"\n "
"\n #endif"
;
static const char *WORLD_SHADER =
"\n uniform mat4 model;"
"\n uniform mat4 view;"
"\n uniform mat4 projection;"
"\n uniform sampler2D tex;"
"\n "
"\n v2f vec3 v_normal;"
"\n v2f vec3 v_worldPos;"
"\n "
"\n #ifdef VERTEX"
"\n "
"\n     layout(location = 0) in vec3 position;"
"\n     layout(location = 1) in vec3 normal;"
"\n "
"\n     void main()"
"\n     {"
"\n         v_normal = inverse(mat3(model)) * normal;"
"\n         vec4 worldPos4 = model * vec4(position, 1.);"
"\n         v_worldPos = worldPos4.xyz;"
"\n         gl_Position = projection * view * worldPos4;"
"\n     }"
"\n "
"\n #endif"
"\n #ifdef FRAGMENT"
"\n "
"\n     vec3 tri( vec3 x )"
"\n     {"
"\n         return abs(x-floor(x)-.5);"
"\n     } "
"\n     float surfFunc( vec3 p )"
"\n     {"
"\n         float n = dot(tri(p*.15 + tri(p.yzx*.075)), vec3(.444));"
"\n         p = p*1.5773 - n;"
"\n         p.yz = vec2(p.y + p.z, p.z - p.y) * .866;"
"\n         p.xz = vec2(p.x + p.z, p.z - p.x) * .866;"
"\n         n += dot(tri(p*.225 + tri(p.yzx*.1125)), vec3(.222));     "
"\n         return abs(n-.5)*1.9 + (1.-abs(sin(n*9.)))*.05;"
"\n     }"
"\n "
"\n     const vec3 light_x = vec3(-1.0, 0.4, 0.9);"
"\n "
"\n     out vec4 color;"
"\n "
"\n     void main() "
"\n     {"
"\n         float surfy = surfFunc( v_worldPos / 50. );"
"\n         float brightness = smoothstep( .2, .3, surfy );"
"\n "
"\n         color = vec4( (0.5 + 0.25 * brightness) * (.5+.5*v_normal), 1 );"
"\n     }"
"\n "
"\n #endif"
;

uint8_t *utils_read_file_alloc( const char *path, size_t *fileLength )
{
    FILE *f = fopen( path, "rb" );

    if( !f ) return NULL;

    fseek( f, 0, SEEK_END );
    size_t length = (size_t)ftell( f );
    rewind( f );
    uint8_t *buffer = malloc( length + 1 );
    fread( buffer, 1, length, f );
    buffer[length] = 0;
    fclose( f );

    if( fileLength ) *fileLength = length;

    return buffer;
}

static GLuint shader_compile( const char *shaderContents, size_t shaderContentsLength, GLenum shaderType )
{
    const GLchar *shaderDefine = shaderType == GL_VERTEX_SHADER 
        ? "\n#version 410\n#define VERTEX  \n#define v2f out\n" 
        : "\n#version 410\n#define FRAGMENT\n#define v2f in \n";

    const GLchar *shaderStrings[2] = { shaderDefine, shaderContents };
    GLint shaderStringLengths[2] = { strlen( shaderDefine ), (GLint)shaderContentsLength };

    GLuint shader = glCreateShader( shaderType );
    glShaderSource( shader, 2, shaderStrings, shaderStringLengths );
    glCompileShader( shader );

    GLint isCompiled;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &isCompiled );
    if( isCompiled == GL_FALSE ) 
    {
        GLint maxLength;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
        char *log = (char*)malloc( maxLength );
        glGetShaderInfoLog( shader, maxLength, &maxLength, log );

        printf( "Error in shader: %s\n%s\n%s\n", log, shaderStrings[0], shaderStrings[1] );
        exit( 1 );
    }

    return shader;
}

static GLuint shader_load( const char *shaderContents )
{
    GLuint result;
    GLuint vert = shader_compile( shaderContents, strlen( shaderContents ), GL_VERTEX_SHADER );
    GLuint frag = shader_compile( shaderContents, strlen( shaderContents ), GL_FRAGMENT_SHADER );

    GLuint ref = glCreateProgram();
    glAttachShader( ref, vert );
    glAttachShader( ref, frag );
    glLinkProgram ( ref );
    glDetachShader( ref, vert );
    glDetachShader( ref, frag );
    result = ref;

    return result;
}

static void load_collision_mesh( CollisionMesh *mesh )
{
    mesh->num_vertices = 3 * surfaces_count;
    mesh->position = malloc( sizeof( float ) * surfaces_count * 9 );
    mesh->normal = malloc( sizeof( float ) * surfaces_count * 9 );
    mesh->index = malloc( sizeof( uint16_t ) * surfaces_count * 3 );

    for( size_t i = 0; i < surfaces_count; ++i )
    {
        const struct SM64Surface *surf = &surfaces[i];

        float x1 = mesh->position[9*i+0] = surf->vertices[0][0];
        float y1 = mesh->position[9*i+1] = surf->vertices[0][1];
        float z1 = mesh->position[9*i+2] = surf->vertices[0][2];
        float x2 = mesh->position[9*i+3] = surf->vertices[1][0];
        float y2 = mesh->position[9*i+4] = surf->vertices[1][1];
        float z2 = mesh->position[9*i+5] = surf->vertices[1][2];
        float x3 = mesh->position[9*i+6] = surf->vertices[2][0];
        float y3 = mesh->position[9*i+7] = surf->vertices[2][1];
        float z3 = mesh->position[9*i+8] = surf->vertices[2][2];

        float nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
        float ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
        float nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
        float mag = sqrtf(nx * nx + ny * ny + nz * nz);
        nx /= mag;
        ny /= mag;
        nz /= mag;

        mesh->normal[9*i+0] = nx;
        mesh->normal[9*i+1] = ny;
        mesh->normal[9*i+2] = nz;
        mesh->normal[9*i+3] = nx;
        mesh->normal[9*i+4] = ny;
        mesh->normal[9*i+5] = nz;
        mesh->normal[9*i+6] = nx;
        mesh->normal[9*i+7] = ny;
        mesh->normal[9*i+8] = nz;

        mesh->index[3*i+0] = 3*i+0;
        mesh->index[3*i+1] = 3*i+1;
        mesh->index[3*i+2] = 3*i+2;
    }

    glGenVertexArrays( 1, &mesh->vao );
    glBindVertexArray( mesh->vao );

    #define X( loc, buff, arr, type ) do { \
        glGenBuffers( 1, &buff ); \
        glBindBuffer( GL_ARRAY_BUFFER, buff ); \
        glBufferData( GL_ARRAY_BUFFER, mesh->num_vertices*sizeof( type ), arr, GL_STATIC_DRAW ); \
        glEnableVertexAttribArray( loc ); \
        glVertexAttribPointer( loc, sizeof( type ) / sizeof( float ), GL_FLOAT, GL_FALSE, sizeof( type ), NULL ); \
    } while( 0 )

        X( 0, mesh->position_buffer, mesh->position, vec3 );
        X( 1, mesh->normal_buffer,   mesh->normal,   vec3 );

    #undef X
}

static void load_mario_mesh( MarioMesh *mesh, struct SM64MarioGeometryBuffers *marioGeo )
{
    mesh->index = malloc( 3 * SM64_GEO_MAX_TRIANGLES * sizeof(uint16_t) );
    for( int i = 0; i < 3 * SM64_GEO_MAX_TRIANGLES; ++i )
        mesh->index[i] = i;

    mesh->num_vertices = 3 * SM64_GEO_MAX_TRIANGLES;

    glGenVertexArrays( 1, &mesh->vao );
    glBindVertexArray( mesh->vao );

    #define X( loc, buff, arr, type ) do { \
        glGenBuffers( 1, &buff ); \
        glBindBuffer( GL_ARRAY_BUFFER, buff ); \
        glBufferData( GL_ARRAY_BUFFER, sizeof( type ) * 3 * SM64_GEO_MAX_TRIANGLES, arr, GL_DYNAMIC_DRAW ); \
        glEnableVertexAttribArray( loc ); \
        glVertexAttribPointer( loc, sizeof( type ) / sizeof( float ), GL_FLOAT, GL_FALSE, sizeof( type ), NULL ); \
    } while( 0 )

        X( 0, mesh->position_buffer, marioGeo->position, vec3 );
        X( 1, mesh->normal_buffer,   marioGeo->normal,   vec3 );
        X( 2, mesh->color_buffer,    marioGeo->color,    vec3 );
        X( 3, mesh->uv_buffer,       marioGeo->uv,       vec2 );

    #undef X
}

static void update_mario_mesh( MarioMesh *mesh, struct SM64MarioGeometryBuffers *marioGeo )
{
    if( mesh->index == NULL )
        load_mario_mesh( mesh, marioGeo );

    mesh->num_vertices = 3 * marioGeo->numTrianglesUsed;

    glBindBuffer( GL_ARRAY_BUFFER, mesh->position_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vec3 ) * 3 * SM64_GEO_MAX_TRIANGLES, marioGeo->position, GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, mesh->normal_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vec3 ) * 3 * SM64_GEO_MAX_TRIANGLES, marioGeo->normal, GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, mesh->color_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vec3 ) * 3 * SM64_GEO_MAX_TRIANGLES, marioGeo->color, GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, mesh->uv_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vec2 ) * 3 * SM64_GEO_MAX_TRIANGLES, marioGeo->uv, GL_DYNAMIC_DRAW );
}

void render_state_init( RenderState *renderState, uint8_t *marioTexture )
{
    load_collision_mesh( &renderState->collision );
    renderState->world_shader = shader_load( WORLD_SHADER );
    renderState->mario_shader = shader_load( MARIO_SHADER );

    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
    glEnable( GL_DEPTH_TEST );

    glGenTextures( 1, &renderState->mario_texture );
    glBindTexture( GL_TEXTURE_2D, renderState->mario_texture );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SM64_TEXTURE_WIDTH, SM64_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, marioTexture);
}

void render_draw( RenderState *renderState, const vec3 camPos, const struct SM64MarioState *marioState, struct SM64MarioGeometryBuffers *marioGeo )
{
    update_mario_mesh( &renderState->mario, marioGeo );

    mat4 model, view, projection;
    glm_perspective( 45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 100.0f, 20000.0f, projection );
    glm_translate( view, (float*)camPos );
    glm_lookat( (float*)camPos, (float*)marioState->position, (vec3){0,1,0}, view );
    glm_mat4_identity( model );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( renderState->world_shader );
    glBindVertexArray( renderState->collision.vao );
    glUniformMatrix4fv( glGetUniformLocation( renderState->world_shader, "model" ), 1, GL_FALSE, (GLfloat*)model );
    glUniformMatrix4fv( glGetUniformLocation( renderState->world_shader, "view" ), 1, GL_FALSE, (GLfloat*)view );
    glUniformMatrix4fv( glGetUniformLocation( renderState->world_shader, "projection" ), 1, GL_FALSE, (GLfloat*)projection );
    glDrawElements( GL_TRIANGLES, renderState->collision.num_vertices, GL_UNSIGNED_SHORT, renderState->collision.index );

    glUseProgram( renderState->mario_shader );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, renderState->mario_texture );
    glBindVertexArray( renderState->mario.vao );
    glUniformMatrix4fv( glGetUniformLocation( renderState->mario_shader, "view" ), 1, GL_FALSE, (GLfloat*)view );
    glUniformMatrix4fv( glGetUniformLocation( renderState->mario_shader, "projection" ), 1, GL_FALSE, (GLfloat*)projection );
    glUniform1i( glGetUniformLocation( renderState->mario_shader, "marioTex" ), 0 );
    glDrawElements( GL_TRIANGLES, renderState->mario.num_vertices, GL_UNSIGNED_SHORT, renderState->mario.index );
}

static float read_axis( int16_t val )
{
    float result = (float)val / 32767.0f;

    if( result < 0.2f && result > -0.2f )
        return 0.0f;

    return result > 0.0f ? (result - 0.2f) / 0.8f : (result + 0.2f) / 0.8f;
}

int main( void )
{
    size_t romSize;

    uint8_t *rom = utils_read_file_alloc( "baserom.us.z64", &romSize );

    if( rom == NULL )
    {
        printf("\nFailed to read ROM file \"baserom.us.z64\"\n\n");
        return 1;
    }

    uint8_t *texture = malloc( 4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT );

    sm64_global_terminate();
    sm64_global_init( rom, texture, NULL );
    sm64_static_surfaces_load( surfaces, surfaces_count );
    uint32_t marioId = sm64_mario_create( 0, 1000, 0 );

    free( rom );

    RenderState renderState;
    renderState.mario.index = NULL;
    vec3 cameraPos = { 0, 0, 0 };
    float cameraRot = 0.0f;

    context_init( "libsm64", WINDOW_WIDTH, WINDOW_HEIGHT );
    render_state_init( &renderState, texture );

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;

    struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;

    marioGeometry.position = malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.color    = malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.normal   = malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.uv       = malloc( sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES );

    do
    {
        uint64_t frameTopTime = ns_clock();

        SDL_GameController *controller = context_get_controller();
        float x_axis = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTX ));
        float y_axis = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTY ));
        float x0_axis = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_RIGHTX ));

        cameraRot += 0.1f * x0_axis;
        cameraPos[0] = marioState.position[0] + 1000.0f * cosf( cameraRot );
        cameraPos[1] = marioState.position[1] + 200.0f;
        cameraPos[2] = marioState.position[2] + 1000.0f * sinf( cameraRot );

        marioInputs.buttonA = SDL_GameControllerGetButton( controller, 0 );
        marioInputs.buttonB = SDL_GameControllerGetButton( controller, 2 );
        marioInputs.buttonZ = SDL_GameControllerGetButton( controller, 9 );
        marioInputs.camLookX = marioState.position[0] - cameraPos[0];
        marioInputs.camLookZ = marioState.position[2] - cameraPos[2];
        marioInputs.stickX = x_axis;
        marioInputs.stickY = y_axis;

        sm64_mario_tick( marioId, &marioInputs, &marioState, &marioGeometry );

        render_draw( &renderState, cameraPos, &marioState, &marioGeometry );

        ts.tv_nsec = 33333333 - (ns_clock() - frameTopTime);
        nanosleep( &ts, &ts );
    }
    while( context_flip_frame_poll_events() );

    sm64_global_terminate();
    context_terminate();

    return 0;
}