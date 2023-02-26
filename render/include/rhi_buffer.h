#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <memory>

struct VBO;
struct VAO;
struct VAO_Attr;
typedef std::shared_ptr<VAO> VAO_ptr;


struct BufferObject {
	GLuint id = 0;
	GLsizeiptr size_bytes = 0;
	GLenum usage = GL_STATIC_DRAW;
	GLenum buffer_type = GL_ARRAY_BUFFER;
	

	BufferObject() {}
	BufferObject(const void* data, GLsizeiptr size_bytes, GLenum usage = GL_STATIC_DRAW, GLenum buffer_type = GL_ARRAY_BUFFER);

	

	void dalete_BO();
};

struct VAO_Attr {
	BufferObject vbo;
	GLuint index = 0;
	GLuint size = 0;
	GLenum type = GL_FLOAT;
	GLsizei stride = 0; // stride = size * sizeof((c)type)
	void* offset = (void*)0; // use offsetof(a, b) when needed
	bool is_instancing = false;
	
	std::string name = "";

	VAO_Attr() {}
	VAO_Attr(const std::string& name, BufferObject& vbo, GLuint index, GLuint attr_size,
		GLenum type = GL_FLOAT, void* offset = (void*)0, bool is_instancing = false);

};



struct VAO {
	GLuint id = 0;
	BufferObject ebo;
	std::vector<VAO_Attr> attributes;
	
	VAO();
	VAO(const VAO&) = delete;
	VAO& operator=(const VAO&) = delete;

	void add_attribute(VAO_Attr&);
	void add_attributes(std::vector<VAO_Attr>&);
	void add_ebo(BufferObject& ebo);
	void use();

	void update_buffer(const std::string& attr_name, void* new_data);
	

	void delete_VAO();
	// now nobody cares about when to delete VAO
};

