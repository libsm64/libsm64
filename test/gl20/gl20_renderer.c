#include "gl20_renderer.h"

#include <stdio.h>

#include "../../src/libsm64.h"
#include "../renderer.h"
#include "../context.h"
#include "../level.h"

GLuint worldTexture;
uint8_t *worldTextureRaw;
int worldTextureSize[2] = {256, 256};
float *worldUv;

static void load_collision_mesh( CollisionMesh *mesh )
{
	mesh->num_vertices = 3 * surfaces_count;
	mesh->position = malloc( sizeof( float ) * surfaces_count * 9 );
	mesh->normal = malloc( sizeof( float ) * surfaces_count * 9 );
	mesh->color = malloc( sizeof( float ) * surfaces_count * 9 );
	worldUv = malloc(sizeof(float) * surfaces_count * 6);
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

		for (int j=0; j<9; j++)
			mesh->color[9*i+j] = (0.5 + 0.25 * 1) * (.5+.5*mesh->normal[9*i+j]);

		mesh->index[3*i+0] = 3*i+0;
		mesh->index[3*i+1] = 3*i+1;
		mesh->index[3*i+2] = 3*i+2;
	}

	for( size_t i = 0; i < surfaces_count/2; i++ )
	{
		worldUv[12*i+0] = 0.f;
		worldUv[12*i+1] = 0.f;
		worldUv[12*i+2] = 4.f;
		worldUv[12*i+3] = 0.f;
		worldUv[12*i+4] = 0.f;
		worldUv[12*i+5] = 4.f;

		worldUv[12*i+6] = 0.f;
		worldUv[12*i+7] = 4.f;
		worldUv[12*i+8] = 4.f;
		worldUv[12*i+9] = 0.f;
		worldUv[12*i+10] = 0.f;
		worldUv[12*i+11] = 0.f;
	}
}

static void load_mario_mesh( MarioMesh *mesh, struct SM64MarioGeometryBuffers *marioGeo )
{
	mesh->index = malloc( 3 * SM64_GEO_MAX_TRIANGLES * sizeof(uint16_t) );
	for( int i = 0; i < 3 * SM64_GEO_MAX_TRIANGLES; ++i )
		mesh->index[i] = i;

	mesh->num_vertices = 3 * SM64_GEO_MAX_TRIANGLES;
}

static void update_mario_mesh( MarioMesh *mesh, struct SM64MarioGeometryBuffers *marioGeo )
{
	if( mesh->index == NULL )
		load_mario_mesh( mesh, marioGeo );

	mesh->num_vertices = 3 * marioGeo->numTrianglesUsed;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, marioGeo->position);
	glNormalPointer(GL_FLOAT, 0, marioGeo->normal);
	glColorPointer(3, GL_FLOAT, 0, marioGeo->color);
	glTexCoordPointer(2, GL_FLOAT, 0, marioGeo->uv);
}

static void gl20_init(RenderState *renderState, uint8_t *marioTexture)
{
	load_collision_mesh( &renderState->collision );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LEQUAL );
	glEnable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
	glColor4f(1,1,1,1);

	glGenTextures( 1, &renderState->mario_texture );
	glBindTexture( GL_TEXTURE_2D, renderState->mario_texture );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SM64_TEXTURE_WIDTH, SM64_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, marioTexture);

	// make a custom world texture programatically
	worldTextureRaw = (uint8_t*)malloc(worldTextureSize[0] * worldTextureSize[1] * 4);
	memset(worldTextureRaw, 255, worldTextureSize[0] * worldTextureSize[1] * 4);

	for (int y=0; y<worldTextureSize[1]/2; y++)
	{
		for (int x = worldTextureSize[1]/2 - (y+1); x<worldTextureSize[1]/2 + (y+1); x++)
		{
			int i1 = y * worldTextureSize[0] + x;
			int i2 = (worldTextureSize[1]-1-y) * worldTextureSize[0] + x;
			worldTextureRaw[i1*4+0] = 192;
			worldTextureRaw[i1*4+1] = 192;
			worldTextureRaw[i1*4+2] = 192;
			worldTextureRaw[i2*4+0] = 192;
			worldTextureRaw[i2*4+1] = 192;
			worldTextureRaw[i2*4+2] = 192;
		}
	}

	glGenTextures(1, &worldTexture );
	glBindTexture(GL_TEXTURE_2D, worldTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, worldTextureSize[0], worldTextureSize[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, worldTextureRaw);
}

static void gl20_draw(RenderState *renderState, const vec3 camPos, const struct SM64MarioState *marioState, struct SM64MarioGeometryBuffers *marioGeo)
{
	mat4 model, view, projection;
	glm_perspective( 45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 100.0f, 20000.0f, projection );
	glm_translate( view, (float*)camPos );
	glm_lookat( (float*)camPos, (float*)marioState->position, (vec3){0,1,0}, view );
	glm_mat4_identity( model );

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((GLfloat*)projection);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((GLfloat*)view);

	glEnable(GL_TEXTURE_2D);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw world
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, renderState->collision.position);
	glNormalPointer(GL_FLOAT, 0, renderState->collision.normal);
	glColorPointer(3, GL_FLOAT, 0, renderState->collision.color);
	glTexCoordPointer(2, GL_FLOAT, 0, worldUv);

	glBindTexture(GL_TEXTURE_2D, worldTexture);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glDrawElements(GL_TRIANGLES, renderState->collision.num_vertices, GL_UNSIGNED_SHORT, renderState->collision.index);

	// create lighting on the scene
	GLfloat light_position[] = { camPos[0], camPos[1], camPos[2], 1 };
	GLfloat light_diffuse[] = { 0.6f, 0.6f, 0.6f, 1 };
	GLfloat light_model[] = { 0.5f, 0.5f, 0.5f, 1 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// first, draw geometry without Mario's texture.
	glBindTexture(GL_TEXTURE_2D, 0);
	update_mario_mesh( &renderState->mario, marioGeo );
	uint32_t triangleSize = renderState->mario.num_vertices;

	glDrawElements(GL_TRIANGLES, triangleSize, GL_UNSIGNED_SHORT, renderState->mario.index);

	// now disable the color array and enable the texture.
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, renderState->mario_texture);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glDrawElements(GL_TRIANGLES, triangleSize, GL_UNSIGNED_SHORT, renderState->mario.index);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
}

struct Renderer gl20_renderer = {
    gl20_init,
    gl20_draw
};
