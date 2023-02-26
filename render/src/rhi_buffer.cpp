#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>

#include "rhi_buffer.h"

#include <glm/glm.hpp>
#include "cu_hack.h"


static size_t sizeof_gltype(GLenum type)
{
	switch (type)
	{
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		return sizeof(GLbyte);
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		return sizeof(GLshort);
	case GL_INT_2_10_10_10_REV:
	case GL_INT:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT:
		return sizeof(GLint);
	case GL_FLOAT:
		return sizeof(GLfloat);
	case GL_DOUBLE:
		return sizeof(GLdouble);
		// case GL_FIXED:
		// 	return sizeof(GLfixed);
	case GL_HALF_FLOAT:
		return sizeof(GLhalf);
	}
	return 0;
}


BufferObject::BufferObject(const void* data, GLsizeiptr size_bytes, GLenum usage, GLenum buffer_type) :
	size_bytes(size_bytes), usage(usage), buffer_type(buffer_type) {
	glGenBuffers(1, &id);
	glBindBuffer(buffer_type, id);
	glBufferData(buffer_type, size_bytes, data, usage);
	glBindBuffer(buffer_type, 0);
}

void BufferObject::dalete_BO() { // what VBO care: delete buffer
	/*
		http://alex-charlton.com/posts/When_is_it_okay_to_delete_an_OpenGL_buffer/
		"
		When a buffer, texture, sampler, renderbuffer, query, or sync object is deleted,
		its name immediately becomes invalid (e.g. is marked unused), but the underlying
		object will not be deleted until it is no longer in use. A buffer, texture,
		sampler, or renderbuffer object is in use if any of the following conditions are
		satisfied:
		- the object is attached to any container object
		- the object is bound to a context bind point in any context
		- any other object contains a view of the data store of the object.
		"
		so glDeleteBuffers is just a hint for users, GL may still use it when vbo is deleted 
		(until all VAO related to it is deleted)
	*/
	glDeleteBuffers(1, &id);
	/*
		If a buffer object that is currently bound is deleted, the binding reverts to 0 (the
		absence of any buffer object).
		glDeleteBuffers silently ignores 0's and names that do not correspond to existing buffer
		objects.
	*/
}


VAO_Attr::VAO_Attr(const std::string& name, BufferObject& vbo, GLuint index, GLuint attr_size,
	GLenum type, void* offset, bool is_instancing) : name(name), index(index), vbo(vbo), size(attr_size),
	type(type), offset(offset), is_instancing(is_instancing)
{
	if (vbo.buffer_type != GL_ARRAY_BUFFER) {
		printf("ERROR: vbo's buffer-type is not GL_ARRAY_BUFFER for VAO_Attr construction\n");
		exit(-1);
	}
	if (vbo.id == 0) {
		printf("ERROR: vbo's id is 0 for VAO_Attr construction\n");
		exit(-1);
	}

	// only compact type allowed
	stride = (GLsizei)size * sizeof_gltype(type);
}

VAO::VAO() {
	glGenVertexArrays(1, &id);
}

void VAO::add_ebo(BufferObject& ebo) {
	if (ebo.buffer_type != GL_ELEMENT_ARRAY_BUFFER) {
		printf("ERROR: ebo's buffer-type is not GL_ELEMENT_ARRAY_BUFFER for VAO construction\n");
		exit(-1);
	}
	glBindVertexArray(id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id);
	glBindVertexArray(0);
}

void VAO::add_attribute(VAO_Attr& attr) {
	this->attributes.push_back(attr);

	glBindVertexArray(id);
	/*
		Note that it is an error to call the glVertexAttribPointer functions if 0 is currently bound
		to GL_ARRAY_BUFFER.
	*/
	glBindBuffer(GL_ARRAY_BUFFER, attr.vbo.id);
	glVertexAttribPointer(attr.index, attr.size, attr.type, GL_FALSE, attr.stride, attr.offset);
	/*
		If divisor is zero, the attribute at slot index advances once per vertex.
		If divisor is non-zero, the attribute advances once per divisor instances of the set(s) of
		vertices being rendered.
	*/
	glVertexAttribDivisor(attr.index, (attr.is_instancing ? 1 : 0));
	glEnableVertexAttribArray(attr.index);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void VAO::add_attributes(std::vector<VAO_Attr>& attrs) {
	for (auto& attr : attrs) {
		add_attribute(attr);
	}
}

void VAO::use() {
	glBindVertexArray(id);
	for (auto& attr : attributes) {
		glEnableVertexAttribArray(attr.index);
	}
}

void VAO::delete_VAO() {
	glDeleteVertexArrays(1, &id);
}

static void _rhi_buffer_updateBuffer(unsigned int& id, unsigned int offset, void* data, unsigned int size, unsigned int type) {
	glBindBuffer(type, id);
	glBufferSubData(type, offset, size, data);
}

void VAO::update_buffer(const std::string& attr_name, void* new_data) {
	glBindVertexArray(id);
	if (attr_name == "face") {
		_rhi_buffer_updateBuffer(ebo.id, 0, new_data, ebo.size_bytes, ebo.buffer_type);
	}
	for (auto& attr : attributes) {
		if (attr.name == attr_name) {
			_rhi_buffer_updateBuffer(attr.vbo.id, 0, new_data, attr.vbo.size_bytes, attr.vbo.buffer_type);
			break;
		}
	}
}