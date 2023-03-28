#include "MarchingCubes.h"
#include "TriTable.hpp"
#include <iostream>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <thread>
#include <mutex>
#include <string>

typedef MarchingCubes::Vertex Vertex;

struct BufferIdentifiers {
	GLuint VAO, VBO;
	int vert_count = 0;
};

glm::vec3 MarchingCubes::base_color = glm::vec3(0, 1, 1);  // Color of the triangles drawn

// Drawing triangles, so each buffer batch must have a multiple of 3 vertices. PUNISHMENT WILL COMMENCE IF THIS ISN'T OBLIGED!
const int VERTS_PER_BATCH = 30000;
const size_t BYTES_PER_BATCH = VERTS_PER_BATCH * sizeof(Vertex);
size_t buffer_bytes_occupied = 0;
int lastIndex = 0;
std::mutex mutex;

std::vector<BufferIdentifiers> buffers; // Groups of VAO and VBO 'batches'
std::vector<Vertex> vertices;

const int LUT_COLUMN_COUNT = 16; // 16 Indexes we could look up in TriTable.hpp

// Computes the normal given 3 vertices, assuming a CCW winding order
glm::vec3 compute_normal(const Vertex& v1, const Vertex& v2,
	const MarchingCubes::Vertex& v3) {
	// v2 - v1 gives vector pointing from v1 to v2, same with v3 - v1.
	glm::vec3 vec12(v2.position - v1.position);
	glm::vec3 vec13(v3.position - v1.position);

	// vec12 x vec13 will give us a a normal positive in CCW order. Make sure its a unit vector.
	return glm::normalize(glm::cross(vec12, vec13));
}

// Create an empty buffer and VAO of a specified size and return those IDS
BufferIdentifiers createEmptyBuffers(int bufferSize) {
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// data must be nullptr since its empty for now.
	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::normal));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	buffer_bytes_occupied = 0; // New empty buffer, so 0 bytes occupied

	return BufferIdentifiers{ VAO, VBO };
}

// BIT MASKS
const int BOT_BACK_LEFT =  0b00000001;
const int BOT_BACK_RIGHT = 0b00000010;
const int BOT_TOP_RIGHT =  0b00000100;
const int BOT_TOP_LEFT =   0b00001000;
const int TOP_BACK_LEFT =  0b00010000;
const int TOP_BACK_RIGHT = 0b00100000;
const int TOP_TOP_RIGHT =  0b01000000;
const int TOP_TOP_LEFT =   0b10000000;

// Populates a vector passed in as an argument.
void marching_cubes(std::function<float(float, float, float)> f, float isovalue,
					float min, float max, float stepsize) {

	int byte_offset = 0; // Used for tracking our offset in glBufferSubData

	// Vertices come in pairs of 3 in the LUT, so we'll do this on a triangle-basis.
	// bot denotes bottom face, top denotes top face (of a cube)
	float bot_bl, bot_br, bot_tr, bot_tl, top_bl, top_br, top_tr, top_tl;
	int marching_case = 0;
	for (float z = min; z < max; z += stepsize)
		for (float x = min; x < max; x += stepsize)
			for (float y = min; y < max; y += stepsize) {
				// Run all vertices of cube through our function, and they have to be less than the isoval
				bot_bl = f(x, y, z);
				bot_br = f(x + stepsize, y, z);
				bot_tr = f(x + stepsize, y, z + stepsize);
				bot_tl = f(x, y, z + stepsize);
				top_bl = f(x, y + stepsize, z);
				top_br = f(x + stepsize, y + stepsize, z);
				top_tr = f(x + stepsize, y + stepsize, z + stepsize);
				top_tl = f(x, y + stepsize, z + stepsize);

				marching_case = 0;

				if (bot_bl < isovalue)
					marching_case |= BOT_BACK_LEFT;
				if (bot_br < isovalue)
					marching_case |= BOT_BACK_RIGHT;
				if (bot_tr < isovalue)
					marching_case |= BOT_TOP_RIGHT;
				if (bot_tl < isovalue)
					marching_case |= BOT_TOP_LEFT;
				if (top_bl < isovalue)
					marching_case |= TOP_BACK_LEFT;
				if (top_br < isovalue)
					marching_case |= TOP_BACK_RIGHT;
				if (top_tr < isovalue)
					marching_case |= TOP_TOP_RIGHT;
				if (top_tl < isovalue)
					marching_case |= TOP_TOP_LEFT;

				// Use the case in the LUT
				int* lut_indices = marching_cubes_lut[marching_case];

				// All entries are 3 vertices at a time to define one triangle, so skip through list in threes
				for (int i = 0; i < LUT_COLUMN_COUNT; i += 3) {

					if (lut_indices[i] < 0)
						break; // Once we hit a -1, we're done with this centry.

					// Vert table 0th index is x, 1st index is y, 2nd index is z.
					// Also scale each by stepsize, in addition yo offsetting yb x,y,z
					Vertex vert1({ x + stepsize * vertTable[lut_indices[i]][0],
								   y + stepsize * vertTable[lut_indices[i]][1],
								   z + stepsize * vertTable[lut_indices[i]][2] },
								   { 0, 0, 0 });
					Vertex vert2({ x + stepsize * vertTable[lut_indices[i + 1]][0],
								   y + stepsize * vertTable[lut_indices[i + 1]][1],
								   z + stepsize * vertTable[lut_indices[i + 1]][2] },
								   { 0, 0, 0 });
					Vertex vert3({x + stepsize * vertTable[lut_indices[i + 2]][0],
								  y + stepsize * vertTable[lut_indices[i + 2]][1],
								  z + stepsize * vertTable[lut_indices[i + 2]][2] },
								  { 0, 0, 0 });

					// Calculate normals here, and all 3 vertices share the same normal.
					glm::vec3 norm = compute_normal(vert1, vert2, vert3);
					vert1.normal = norm;
					vert2.normal = norm;
					vert3.normal = norm;

					// Now add vertices to list (critical section)
					std::lock_guard<std::mutex> lock(mutex);
					vertices.emplace_back(vert1);
					vertices.emplace_back(vert2);
					vertices.emplace_back(vert3);
				}
			}
}

// Writes the vertex information to a ply file, FILENAME SHOULD NOT CONTAIN .PLY
void writeToPLY(std::vector<Vertex>& vertices, std::string filename) {

	std::ofstream outfile((filename+".ply"));
	std::string header;
	std::string vertex_data;
	std::string face_data;

	// BEGIN HEADER
	header =
		"ply\n"
		"format ascii 1.0\n";
	header += "element vertex " + std::to_string(vertices.size()) + "\n";
	header +=
		"property float x\n"
		"property float y\n"
		"property float z\n"
		"property float nx\n"
		"property float ny\n"
		"property float nz\n";
	header += 
		"element face " + std::to_string(vertices.size() / 3) + "\n"
		"property list uchar uint vertex_indices\n"
		"end_header\n";

	// BODY INFORMATION (Vertex)
	for (const Vertex& v : vertices) {
		vertex_data +=  std::to_string(v.position.x) + " " + 
						std::to_string(v.position.y) + " " + 
						std::to_string(v.position.z) + " " + 
						std::to_string(v.normal.x) + " " + 
						std::to_string(v.normal.y) + " " +
						std::to_string(v.normal.z) + "\n";
	}

	// BODY INFORMATION (Face, unique vertices so follow pattern)
	for (int i = 0; i < vertices.size(); i += 3)
		face_data += "3 " + std::to_string(i) + " " + std::to_string(i+1) + " " + std::to_string(i+2) + "\n";

	outfile << header;
	outfile << vertex_data;
	outfile << face_data;

	outfile.close();
}

void MarchingCubes::init(std::function<float(float, float, float)> f, float isovalue, 
	float min, float max, float stepsize) {

	// First, get our vertices from marching cubes asynchronously
	marching_cubes(f, isovalue, min, max, stepsize);

	std::cout << "Writing vertices to file..." << std::endl;
	// When vertices are finished, we can write to a PLY file.
	writeToPLY(vertices, "output");
	std::cout << "Done writing to file." << std::endl;
}

void MarchingCubes::update() {
	std::lock_guard<std::mutex> lock(mutex);
	// If vertices haven't been added, no work needs to be done.
	if (lastIndex == vertices.size())
		return;

	// If we don't have any buffers yet, just create a new one
	if (buffers.size() == 0)
		buffers.emplace_back(createEmptyBuffers(VERTS_PER_BATCH * sizeof(Vertex)));
	
	// The partition of the vector vertex to what the buffer can hold
	std::vector<Vertex> vertex_partition;

	// Add as many vertices as we can, until the buffer is fully occupied and then make a new buffer if so.
	size_t partition_bytes = 0;
	for (int i = lastIndex; i < vertices.size(); i++) {
		vertex_partition.emplace_back(vertices[i]);
		partition_bytes = vertex_partition.size() * sizeof(Vertex);

		// Buffer is ready to be full, time fill it, and make a new one.
		if (buffer_bytes_occupied+partition_bytes >= BYTES_PER_BATCH) {
			glBindVertexArray(buffers.back().VAO);
			glBindBuffer(GL_ARRAY_BUFFER, buffers.back().VBO);
			glBufferSubData(GL_ARRAY_BUFFER, buffer_bytes_occupied,
				partition_bytes, &vertex_partition[0]);

			buffers.back().vert_count += vertex_partition.size();
			buffers.emplace_back(createEmptyBuffers(BYTES_PER_BATCH));

			vertex_partition.clear();
		}
	}

	//Now, add whatever remains in the partition into the buffer
	if (vertex_partition.size() > 0) {
		glBindVertexArray(buffers.back().VAO);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.back().VBO);
		glBufferSubData(GL_ARRAY_BUFFER, buffer_bytes_occupied,
			partition_bytes, &vertex_partition[0]);

		buffers.back().vert_count += vertex_partition.size();
		buffer_bytes_occupied += partition_bytes;
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	lastIndex = vertices.size(); // The index of the next element to be added (Since its [begin, end) in vector splitting)
}

void MarchingCubes::render(ShaderProgram& shader, glm::mat4 mvp) {

	// Draw each buffer that we are able
	for (int i = 0; i < buffers.size(); i++) {
		glUseProgram(shader.ID);
		shader.setUniformMatrix4fv("mvp", mvp);
		shader.setUniform3fv("modelColor", base_color);
		glBindVertexArray(buffers[i].VAO);
		glDrawArrays(GL_TRIANGLES, 0, buffers[i].vert_count);
		glBindVertexArray(0);
	}
}