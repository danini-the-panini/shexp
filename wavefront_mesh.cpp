#include "wavefront_mesh.h"

#include <fstream>
#include <iostream>
#include <sstream>

WavefrontMesh::
WavefrontMesh(const char * filename)
  : _filename(filename)
{
}

WavefrontMesh::
~WavefrontMesh()
{
}

static vector<string> &split(const string &s, char delim, bool includeEmpty, vector<string> &elems)
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        if (includeEmpty || !item.empty()) elems.push_back(item);
    }
    return elems;
}

static vector<string> split(const string &s, char delim = ' ', bool includeEmpty = false)
{
    vector<string> elems;
    split(s, delim, includeEmpty, elems);
    return elems;
}

void WavefrontMesh::
build_impl(float_list &vertices, uint_list &indices)
{
  ifstream in(_filename);
  if (!in)
  {
    cout << "Can't read file " << _filename << endl;
    return;
  }
  string line;

  vector<vec3> normals;
  vector<vec3> positions;
  vector<GLuint> ninds;

  while (in)
  {
    getline(in, line);

    auto e = split(line);
    if (e.empty()) continue;
    if (e[0].compare("v") == 0)
    {
      positions.push_back(vec3(atof(e[1].c_str()) ,atof(e[2].c_str()) ,atof(e[3].c_str())));
    } else if (e[0].compare("vn") == 0)
    {
      normals.push_back(vec3(atof(e[1].c_str()),atof(e[2].c_str()),atof(e[3].c_str())));
    } else if (e[0].compare("f") == 0)
    {
      for (GLuint i = 1; i < e.size(); i++)
      {
        auto vn = split(e[i],'/',true);

        indices.push_back(atoi(vn[0].c_str())-1);
        ninds.push_back(atoi(vn[2].c_str())-1);
      }
    }
  }
  in.close();

  bool* visited = new bool[positions.size()];
  for (GLulong i = 0, end = positions.size(); i < end; i++)
    visited[i] = 0;

  vec3* normals2 = new vec3[normals.size()];

  for (GLulong i = 0, end = indices.size(); i < end; i++)
  {
    GLuint ni = ninds[i];
    GLuint vi = indices[i];
    if (!visited[vi])
    {
      visited[vi] = true;

      normals2[vi] = normals[ni];
    }
  }

  for (GLulong i = 0, end = positions.size(); i < end; i++)
  {
    vertices.push_back(positions[i].x);
    vertices.push_back(positions[i].y);
    vertices.push_back(positions[i].z);

    vertices.push_back(normals2[i].x);
    vertices.push_back(normals2[i].y);
    vertices.push_back(normals2[i].z);
  }
}
