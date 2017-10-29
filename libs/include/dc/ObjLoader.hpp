#pragma once
#include <glm/glm.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <iterator>

#include "Materials.hpp"
#include "VertexData.hpp"
#include "Mesh.hpp"

namespace dc
{
    namespace
    {
        struct ObjFace
        {
            std::vector<unsigned> vertexIndices;
            std::vector<unsigned> texCoordIndices;
            std::vector<unsigned> normalIndices;
        };

        glm::vec2 parse_vec2(const std::string& str)
        {
            std::istringstream iss(str);
            glm::vec2 v;
            if (!(iss >> v.x >> v.y))
                throw std::invalid_argument("unable to parse vec2");
            return v;
        }

        glm::vec3 parse_vec3(const std::string& str)
        {
            std::istringstream iss(str);
            glm::vec3 v;
            if (!(iss >> v.x >> v.y >> v.z))
                throw std::invalid_argument("unable to parse vec3");
            return v;
        }

        template<typename Out>
        void split(const std::string& s, char delim, Out result)
        {
            std::stringstream ss;
            ss.str(s);
            std::string item;
            while (std::getline(ss, item, delim))
                *(result++) = item;
        }

        std::vector<std::string> split(const std::string& s, char delim)
        {
            std::vector<std::string> elems;
            split(s, delim, std::back_inserter(elems));
            return elems;
        }

        ObjFace parse_face(const std::string& str)
        {
            std::istringstream iss(str);
            ObjFace f;

            auto indices = split(str, ' ');
            if (indices.size() != 3)
                throw std::invalid_argument("unable to parse face. not a triangle!");

            for (auto index : indices)
            {
                auto inds = split(index, '/');
                unsigned vi = 0;
                unsigned ti = 0;
                unsigned ni = 0;

                if (inds.size() >= 1)
                    f.vertexIndices.push_back(std::stoi(inds[0]));
                if (inds.size() >= 2 && inds[1].length() > 0)
                    f.texCoordIndices.push_back(std::stoi(inds[1]));
                if (inds.size() == 3)
                    f.normalIndices.push_back(std::stoi(inds[2]));
            }

            return f;
        }

        inline bool starts_with(const std::string& input, const std::string& match)
        {
            return input.length() >= match.length() && (input.compare(0, match.length(), match) == 0);
        }
    }

    class ObjLoader
    {
    public:
        ObjLoader(const std::string& filePath, bool normalizeNormals = true)
        {
            std::ifstream objFile(filePath);

            std::string currentMaterial = "NO_MATERIAL";
            std::string line;
            while (std::getline(objFile, line))
            {
                if (line.length() == 0)
                    continue;
                size_t ind = line.find_first_of(" ");
                std::string cmd = line.substr(0, ind);
                std::string val = line.substr(ind + 1);

                if (cmd == "mtllib")
                {
                    std::string dir = filePath.substr(0, filePath.find_last_of('/') + 1);
                    parseMaterialFile(dir + val);
                }
                else if (cmd == "usemtl")
                {
                    currentMaterial = val;
                }
                else if (cmd == "f")
                {
                    mIndices[currentMaterial].push_back(parse_face(val));
                }
                else if (cmd == "vn")
                {
                    glm::vec3 vn = parse_vec3(val);
                    if (normalizeNormals)
                    {
                        vn = glm::normalize(vn);
                    }
                    mNormals.push_back(vn);
                }
                else if (cmd == "vt")
                {
                    mTexCoords.push_back(parse_vec2(val));
                }
                else if (cmd == "v")
                {
                    mVertices.push_back(parse_vec3(val));
                }
            }
        }

        ~ObjLoader() = default;
        ObjLoader(const ObjLoader& other) = delete;
        ObjLoader& operator=(const ObjLoader& other) = delete;

        std::shared_ptr<dc::Mesh> exportMesh() const
        {
            std::vector<unsigned> indices;
            std::vector<dc::VertexData> vertexData;
            std::vector<dc::IndexGroup> groups;

            std::map<unsigned, unsigned> vertexMap;

            for (auto indexGroup : mIndices)
            {
                std::vector<dc::ObjFace> faces = indexGroup.second;
                unsigned offset = indices.size();
                unsigned count = 0;

                for (auto face : faces)
                {
                    for (unsigned i = 0; i < face.vertexIndices.size(); ++i)
                    {
                        unsigned v = face.vertexIndices[i];
                        unsigned n = (face.normalIndices.size() > i) ? face.normalIndices[i] : 0;
                        unsigned t = (face.texCoordIndices.size() > i) ? face.texCoordIndices[i] : 0;

                        unsigned key = v + n * mVertices.size() + t * mVertices.size() * mNormals.size();
                        if (!vertexMap.count(key))
                        {
                            // add to output list
                            dc::VertexData vertex;
                            vertex.position = mVertices[v - 1];
                            if (n > 0)
                                vertex.normal = mNormals[n - 1];
                            if (t > 0)
                                vertex.texcoord = mTexCoords[t - 1];

                            vertexMap[key] = vertexData.size();
                            vertexData.push_back(vertex);
                        }
                        indices.push_back(vertexMap[key]);
                        ++count;
                    }
                }

                groups.push_back({ offset, count, mMaterials.at(indexGroup.first) });
            }

            return std::make_shared<dc::Mesh>(vertexData, indices, groups);
        }

    private:

        std::vector<glm::vec3> mVertices;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec2> mTexCoords;
        std::map<std::string, std::vector<ObjFace>> mIndices;
        std::map<std::string, ObjMaterial> mMaterials;

        void parseMaterialFile(const std::string& filePath)
        {
            std::ifstream matFile(filePath);
            std::string dir = filePath.substr(0, filePath.find_last_of('/') + 1);

            std::string currentMaterial = "NO_MATERIAL";
            std::string line;
            while (std::getline(matFile, line))
            {
                if (line.length() == 0)
                    continue;
                size_t ind = line.find_first_of(" ");
                std::string cmd = line.substr(0, ind);
                std::string val = line.substr(ind + 1);

                if (cmd == "newmtl")
                {
                    currentMaterial = val;
                    mMaterials[currentMaterial] = dc::ObjMaterial();
                }
                else if (cmd == "Ka")
                {
                    mMaterials[currentMaterial].Ka = parse_vec3(val);
                }
                else if (cmd == "Kd")
                {
                    mMaterials[currentMaterial].Kd = parse_vec3(val);
                }
                else if (cmd == "Ks")
                {
                    mMaterials[currentMaterial].Ks = parse_vec3(val);
                }
                else if (cmd == "map_Kd")
                {
                    mMaterials[currentMaterial].map_Kd = dir + val;
                }
            }
        }
    };
}