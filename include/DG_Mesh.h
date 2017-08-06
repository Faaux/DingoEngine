#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>

#include "DG_Transform.h"
#include "DG_Primitives.h"
#include "DG_Shader.h"

namespace DG
{
	class Primitive
	{

	public:

		Primitive() 
		{
		};
		virtual ~Primitive() {};

		Transform transform;

	protected:

		GLuint VAO;
		GLuint VBO_points;
		GLuint VBO_colors;

	};

	class Cube : public Primitive
	{
	public:

		Cube()
		{
			VAO = 0;
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO_points);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_points);
			glBufferData(GL_ARRAY_BUFFER, sizeof(primitives::cube), primitives::cube, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

			glGenBuffers(1, &VBO_colors);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
			glBufferData(GL_ARRAY_BUFFER, sizeof(primitives::cube_color), primitives::cube_color, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		}

		void bind()
		{
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glBindVertexArray(VAO);

		}

		void draw()
		{
			bind();
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	};

}



#endif
