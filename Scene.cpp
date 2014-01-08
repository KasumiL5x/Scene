/*
  Scene is a custom 3d scene parser intended for use with graphical demos.
  
  Copyright (C) 2013, Daniel Green

  Scene is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Scene is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Scene.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "Scene.hpp"
#include "StringUtils.hpp"

Scene::Scene()
  : _parserState(kParserStateWhitespace) {
}

Scene::~Scene() {
}

bool Scene::load( const std::string& file ) {
  // Clean the Scene so it's nice and fresh.
  clean();

  // Open the file.
  FILE* const f = fopen(file.c_str(), "rb");
  // NOTE: Comment out the above line and uncomment the two lines below if using Visual Studio.  Microsoft seem to have
  //       a penchant for not using the right syntax.
  //FILE* f = nullptr;
  //f = std::fopen(file.c_str(), "rb");

  // If it failed to open, return.
  if( f == nullptr ) {
    return false;
  }

  // Get the size of the file.
  fseek(f, 0, SEEK_END);
  const long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  // If there's no size, return.
  if( size == 0 ) {
    fclose(f);
    return false;
  }

  // Read the entire file into a big buffer. Buffer should be one extra character to
  // enforce a null terminator, as fread apparently ignores them?
  char* buffer = new char[size+1];
  memset(buffer, 0, sizeof(char) * size);
  fread(buffer, sizeof(char), size, f);
  // Append null terminator.
  buffer[size] = '\0';

  // Object that will be filled until complete, and then copied into the vector and reset.
  Object tmpObject;
  tmpObject.reset();
  // Texture that will be filled until complete, and then copied into the vector and reset.
  Texture tmpTexture;
  tmpTexture.reset();
  // Mesh that will be filled until complete, and then copied into the vector and reset.
  Mesh tmpMesh;
  tmpMesh.reset();
  // Material that will be filled until complete, and then copied into the vector and reset.
  Material tmpMaterial;
  tmpMaterial.reset();
  // Light that will be filled until complete, and then copied into the vector and reset.
  Light tmpLight;
  tmpLight.reset();

  // Parse the entire buffer, line-by-line.
  long index = 0;
  long end   = 0;
  char c     = ' ';
  while( true ) {
    // Safety check for size.
    if( index > size ) {
      break;
    }

    // Read the current character.
    c = buffer[index];

    // EoF check.
    if( c == '\0' ) {
      break;
    }

    // Update the end to the current index.
    end = index;

    // Read until we get a new line character.
    while( true ) {
      // Read the newest char.
      c = buffer[end];
      // If this char is a newline character, break out.
      // Also check EoF just in case we never get a new line.
      if( c == '\r' || c == '\n' || c == '\0' ) {
        break;
      }
      // Increment to the next char as we haven't found newline or EoF yet.
      end += 1;
    }

    // Check the size of the string we're about to read is > 0.  If not, continue on.
    const long len = end - index;
    if( len <= 0 ) {
      index += 1;
      continue;
    }

    // Read the current range into a string.
    const std::string currLine(&buffer[index], len);
    // Parse the line!
    parseLine(currLine, tmpObject, tmpTexture, tmpMesh, tmpMaterial, tmpLight);

    // Set the new index to the end character (new line) + 1 to read the next character.=
    index = end + 1;
  }

  // Clean up the buffer.
  delete[] buffer;
  buffer = nullptr;
  
  // Close the file.
  fclose(f);

  return true;
}

void Scene::debugOutput() const {
  // Output Textures.
  std::clog << "Textures: " << _textures.size() << std::endl;
  for( unsigned int i = 0; i < _textures.size(); ++i ) {
    std::clog << "[" << i << "].file = " << _textures[i].file.c_str() << std::endl;
    std::clog << "[" << i << "].name = " << _textures[i].name.c_str() << std::endl;
  }
  std::clog << std::endl;

  // Output Meshes.
  std::clog << "Meshes: " << _meshes.size() << std::endl;
  for( unsigned int i = 0; i < _meshes.size(); ++i ) {
    std::clog << "[" << i << "].file = " << _meshes[i].file.c_str() << std::endl;
    std::clog << "[" << i << "].name = " << _meshes[i].name.c_str() << std::endl;
  }
  std::clog << std::endl;

  // Output Materials.
  std::clog << "Materials: " << _materials.size() << std::endl;
  for( unsigned int i = 0; i < _materials.size(); ++i ) {
    const Material& mat = _materials[i];
    std::clog << "[" << i << "].name = " << mat.name.c_str() << std::endl;
    std::clog << "[" << i << "].color = " << mat.color.x << ", " << mat.color.y << ", " << mat.color.z << std::endl;
    std::clog << "[" << i << "].specSize = " << mat.specSize << std::endl;
    std::clog << "[" << i << "].diffuseTex = " << mat.diffuseTex << std::endl;
    std::clog << "[" << i << "].normalTex = " << mat.normalTex << std::endl;
  }
  std::clog << std::endl;

  // Output all Objects.
  std::clog << "Objects: " << _objects.size() << std::endl;
  for( unsigned int i = 0; i < _objects.size(); ++i ) {
    const Object& obj = _objects[i];

    std::clog << "[" << i << "].name = " << obj.name.c_str() << std::endl;
    std::clog << "[" << i << "].position = " << obj.position.x << ", " << obj.position.y << ", " << obj.position.z << std::endl;
    std::clog << "[" << i << "].orientation = " << obj.orientation.x << ", " << obj.orientation.y << ", " << obj.orientation.z << std::endl;
    std::clog << "[" << i << "].scale = " << obj.scale.x << ", " << obj.scale.y << ", " << obj.scale.z << std::endl;
    std::clog << "[" << i << "].mesh = " << obj.mesh << std::endl;
    std::clog << "[" << i << "].material = " << obj.material << std::endl;

    std::clog << std::endl;
  }

  // Output all Lights.
  std::clog << "Lights: " << _lights.size() << std::endl;
  for( unsigned int i = 0; i < _lights.size(); ++i ) {
    const Light& light = _lights[i];

    std::clog << "[" << i << "].type = " << light.type << std::endl;
    std::clog << "[" << i << "].diffuseColor = " << light.diffuseColor.x << ", " << light.diffuseColor.y << ", " << light.diffuseColor.z << std::endl;
    std::clog << "[" << i << "].diffuseIntensity = " << light.diffuseIntensity << std::endl;
    std::clog << "[" << i << "].specularColor = " << light.specularColor.x << ", " << light.specularColor.y << ", " << light.specularColor.z << std::endl;
    std::clog << "[" << i << "].specularIntensity = " << light.specularIntensity << std::endl;
    std::clog << "[" << i << "].position = " << light.position.x << ", " << light.position.y << ", " << light.position.z << std::endl;
    std::clog << "[" << i << "].range = " << light.range << std::endl;
    std::clog << "[" << i << "].direction = " << light.direction.x << ", " << light.direction.y << ", " << light.direction.z << std::endl;
    std::clog << "[" << i << "].shadows = " << ((light.shadows) ? "true" : "false") << std::endl;
    std::clog << "[" << i << "].shadowBias = " << light.shadowBias << std::endl;
    std::clog << "[" << i << "].coneInnerAngle = " << light.coneInnerAngle << std::endl;
    std::clog << "[" << i << "].coneOuterAngle = " << light.coneOuterAngle << std::endl;

    std::clog << std::endl;
  }
}

const std::vector<Scene::Object>& Scene::objects() const {
  return _objects;
}

const std::vector<Scene::Texture>& Scene::textures() const {
  return _textures;
}

const std::vector<Scene::Mesh>& Scene::meshes() const {
  return _meshes;
}

const std::vector<Scene::Material>& Scene::materials() const {
  return _materials;
}

const std::vector<Scene::Light>& Scene::lights() const {
  return _lights;
}

unsigned int Scene::objectCount() const {
  return _objects.size();
}

unsigned int Scene::textureCount() const {
  return _textures.size();
}

unsigned int Scene::meshCount() const {
  return _meshes.size();
}

unsigned int Scene::materialCount() const {
  return _materials.size();
}

unsigned int Scene::lightCount() const {
  return _lights.size();
}

void Scene::parseLine( const std::string& line, Object& obj, Texture& tex, Mesh& mesh, Material& mat, Light& light ) {

  // Split excess whitespace from the line.
  const std::string& newLine = strutils::removeSpaces(line);

  // Check for comment.
  if( newLine[0] == '/' ) {
    // If there's only one character, it's still technically a comment (but broken), so ignore it.
    if( newLine.size() < 2 ) {
      return;
    }

    // Check for a single line comment (//).
    if( newLine[1] == '/' ) {
      // Ignore this line.
      return;
    }
  }


  // Switch the current state of the parser.
  switch( _parserState ) {
    case kParserStateWhitespace: {
      // Check for the beginning of a scene.
      if( strcmp(newLine.c_str(), "[scene]") == 0 ) {
        // We are now inside a scene.
        _parserState = kParserStateScene;
        break;
      }

      break;
    }

    case kParserStateScene: {
      // Check for [resources].
      if( strcmp(newLine.c_str(), "[resources]") == 0 ) {
        _parserState = kParserStateResources;
        break;
      }

      // Check for [objects].
      if( strcmp(newLine.c_str(), "[objects]") == 0 ) {
        _parserState = kParserStateObjects;
        break;
      }

      // Check for [lights].
      if( strcmp(newLine.c_str(), "[lights]") == 0 ) {
        _parserState = kParserStateLights;
        break;
      }

      // Check for end.
      if( strcmp(newLine.c_str(), "[/scene]") == 0 ) {
        // Go back to whitespace mode.
        _parserState = kParserStateWhitespace;
        break;
      }

      // Split the string via '='.
      std::vector<std::string> split;
      strutils::splitString(newLine.c_str(), newLine.size(), '=', false, &split);

      // If there's less than two splits, don't continue.
      if( split.size() < 2 ) {
        break;
      }

      break;
    }

    case kParserStateResources: {
      // Check for [texture].
      if( strcmp(newLine.c_str(), "[texture]") == 0 ) {
        _parserState = kParserStateResourceTexture;
        break;
      }

      // Check for [mesh].
      if( strcmp(newLine.c_str(), "[mesh]") == 0 ) {
        _parserState = kParserStateResourceMesh;
        break;
      }

      // Check for [material].
      if( strcmp(newLine.c_str(), "[material]") == 0 ) {
        _parserState = kParserStateResourceMaterial;
        break;
      }

      // Check for end.
      if( strcmp(newLine.c_str(), "[/resources]") == 0 ) {
        // Go back to [scene].
        _parserState = kParserStateScene;
        break;
      }

      break;
    }

    case kParserStateResourceTexture: {
      // Check for end.
      if( strcmp(newLine.c_str(), "[/texture]") == 0 ) {
        // Add the Texture to the vector and reset it.
        _textures.push_back(tex);
        tex.reset();

        _parserState = kParserStateResources;
        break;
      }

      // Split the string via '='.
      std::vector<std::string> split;
      strutils::splitString(newLine.c_str(), newLine.size(), '=', false, &split);

      // If there's less than two splits, don't continue.
      if( split.size() < 2 ) {
        break;
      }

      // Check for file.
      if( strcmp(split[0].c_str(), "file") == 0 ) {
        tex.file = split[1].c_str();
        break;
      }

      // Check for name.
      if( strcmp(split[0].c_str(), "name") == 0 ) {
        tex.name = split[1].c_str();
        break;
      }

      break;
    }

    case kParserStateResourceMesh: {
      // Check for end.
      if( strcmp(newLine.c_str(), "[/mesh]") == 0 ) {
        // Add the Mesh to the vector and reset it.
        _meshes.push_back(mesh);
        mesh.reset();

        _parserState = kParserStateResources;
        break;
      }

      // Split the string via '='.
      std::vector<std::string> split;
      strutils::splitString(newLine.c_str(), newLine.size(), '=', false, &split);

      // If there's less than two splits, don't continue.
      if( split.size() < 2 ) {
        break;
      }

      // Check for file.
      if( strcmp(split[0].c_str(), "file") == 0 ) {
        mesh.file = split[1];
        break;
      }

      // Check for name.
      if( strcmp(split[0].c_str(), "name") == 0 ) {
        mesh.name = split[1];
        break;
      }

      break;
    }

    case kParserStateResourceMaterial: {
      // Check for end.
      if( strcmp(newLine.c_str(), "[/material]") == 0 ) {
        // Add the Material to the materials vector and reset it.
        _materials.push_back(mat);
        mat.reset();

        // Back to resources.
        _parserState = kParserStateResources;
        break;
      }

      // Split the string via '='.
      std::vector<std::string> split;
      strutils::splitString(newLine.c_str(), newLine.size(), '=', false, &split);

      // If there's less than two splits, don't continue.
      if( split.size() < 2 ) {
        break;
      }

      // Check for name.
      if( strcmp(split[0].c_str(), "name") == 0 ) {
        mat.name = split[1];
        break;
      }

      if( strcmp(split[0].c_str(), "color") == 0 ) {
        readVector(split[1], &mat.color);
        break;
      }

      if( strcmp(split[0].c_str(), "specSize") == 0 ) {
        mat.specSize = atof(split[1].c_str());
        break;
      }

      if( strcmp(split[0].c_str(), "diffuseTex") == 0 ) {
        mat.diffuseTex = findTextureIndex(split[1]);
        break;
      }

      if( strcmp(split[0].c_str(), "normalTex") == 0 ) {
        mat.normalTex = findTextureIndex(split[1]);
        break;
      }

      break;
    }

    case kParserStateObjects: {
      if( strcmp(newLine.c_str(), "[obj]") == 0 ) {
        _parserState = kParserStateObjectsObj;
        break;
      }

      // Check for end.
      if( strcmp(newLine.c_str(), "[/objects]") == 0 ) {
        // Go back to [scene].
        _parserState = kParserStateScene;
        break;
      }

      break;
    }

    case kParserStateObjectsObj: {
      // Check for end.
      if( strcmp(newLine.c_str(), "[/obj]") == 0 ) {
        // Go back to scene.
        _parserState = kParserStateObjects;
        // Add the Object to the list.
        _objects.push_back(obj);
        // Reset the Object for the next parsing.
        obj.reset();
        break;
      }

      // Split the string via '='.
      std::vector<std::string> split;
      strutils::splitString(newLine.c_str(), newLine.size(), '=', false, &split);

      // If there's less than two splits, don't continue.
      if( split.size() < 2 ) {
        break;
      }

      // Check for name.
      if( strcmp(split[0].c_str(), "name") == 0 ) {
        obj.name = split[1];
        break;
      }

      // Check for position.
      if( strcmp(split[0].c_str(), "position") == 0 ) {
        readVector(split[1], &obj.position);
        break;
      }

      // Check for orientation.
      if( strcmp(split[0].c_str(), "orientation") == 0 ) {
        readVector(split[1].c_str(), &obj.orientation);
        break;
      }

      // Check for scale.
      if( strcmp(split[0].c_str(), "scale") == 0 ) {
        readVector(split[1].c_str(), &obj.scale);
        break;
      }

      // Check for mesh.
      if( strcmp(split[0].c_str(), "mesh") == 0 ) {
        obj.mesh = findMeshIndex(split[1]);
        break;
      }

      // Check for material.
      if( strcmp(split[0].c_str(), "material") == 0 ) {
        obj.material = findMaterialIndex(split[1].c_str());
      }

      break;
    }

    case kParserStateLights: {
      if( strcmp(newLine.c_str(), "[light]") == 0 ) {
        _parserState = kParserStateLightsLight;
        break;
      }

      // Check for end.
      if( strcmp(newLine.c_str(), "[/lights]") == 0 ) {
        // Go back to scene.
        _parserState = kParserStateScene;
        break;
      }

      break;
    }

    case kParserStateLightsLight: {
      // Check for end.
      if( strcmp(newLine.c_str(), "[/light]") == 0 ) {
        // Go back to Lights.
        _parserState = kParserStateLights;
        // Add the light.
        _lights.push_back(light);
        // Reset the light.
        light.reset();
        break;
      }

      // Split the string via '='.
      std::vector<std::string> split;
      strutils::splitString(newLine.c_str(), newLine.size(), '=', false, &split);

      // If there's less than two splits, don't continue.
      if( split.size() < 2 ) {
        break;
      }

      // Check for type.
      if( strcmp(split[0].c_str(), "type") == 0 ) {
        // Check for type of light.
        if( strcmp(split[1].c_str(), "point") == 0 ) {
          light.type = kLightTypePoint;
        } else if( strcmp(split[1].c_str(), "spot") == 0 ) {
          light.type = kLightTypeSpot;
        } else if( strcmp(split[1].c_str(), "directional") == 0 ) {
          light.type = kLightTypeDirectional;
        }
        break;
      }

      // Check for diffuse color.
      if( strcmp(split[0].c_str(), "diffuseColor") == 0 ) {
        readVector(split[1], &light.diffuseColor);
        break;
      }

      // Check for diffuse intensity.
      if( strcmp(split[0].c_str(), "diffuseIntensity") == 0 ) {
        light.diffuseIntensity = atof(split[1].c_str());
        break;
      }

      // Check for specular color.
      if( strcmp(split[0].c_str(), "specularColor") == 0 ) {
        readVector(split[1].c_str(), &light.specularColor);
        break;
      }

      // Check for specular intensity.
      if( strcmp(split[0].c_str(), "specularIntensity") == 0 ){
        light.specularIntensity = atof(split[1].c_str());
        break;
      }

      // Check for position.
      if( strcmp(split[0].c_str(), "position") == 0 ) {
        readVector(split[1], &light.position);
        break;
      }

      // Check for range.
      if( strcmp(split[0].c_str(), "range") == 0 ) {
        light.range = atof(split[1].c_str());
        break;
      }

      // Check for direction.
      if( strcmp(split[0].c_str(), "direction") == 0 ) {
        readVector(split[1], &light.direction);
        break;
      }

      // Check for shadows.
      if( strcmp(split[0].c_str(), "shadows") == 0 ) {
        parseBool(split[1], &light.shadows);
        break;
      }

      // Check for shadow bias.
      if( strcmp(split[0].c_str(), "shadowBias") == 0 ){
        light.shadowBias = atof(split[1].c_str());
        break;
      }

      // Check for cone inner angle.
      if( strcmp(split[0].c_str(), "coneInnerAngle") == 0 ) {
        light.coneInnerAngle = atof(split[1].c_str());
        break;
      }
      
      // Check for cone outer angle.
      if( strcmp(split[0].c_str(), "coneOuterAngle") == 0 ) {
        light.coneOuterAngle = atof(split[1].c_str());
        break;
      }

      break;
    }

    default: {
      break;
    }
  }
}

void Scene::readVector( const std::string& line, Vector* outVec ) const {
  // Separate the string by ','.
  std::vector<std::string> split;
  strutils::splitString(line.c_str(), line.size(), ',', false, &split);

  // Safety check - should be three components.
  if( split.size() != 3 ) {
    return;
  }

  // Parse X, Y, and Z.
  outVec->x = atof(split[0].c_str());
  outVec->y = atof(split[1].c_str());
  outVec->z = atof(split[2].c_str());
}

void Scene::parseBool( const std::string& value, bool* out ) {
  if( out == nullptr ) {
    return;
  }

  *out = (strcmp(value.c_str(), "true") == 0) ||
         (strcmp(value.c_str(), "yes")  == 0) ||
         (strcmp(value.c_str(), "1")    == 0);
}

int Scene::findTextureIndex( const std::string& file ) const {
  for( int i = 0; i < _textures.size(); ++i ) {
    if( strcmp(file.c_str(), _textures[i].name.c_str()) != 0 ) {
      continue;
    }
    return i;
  }
  return -1;
}

int Scene::findMeshIndex( const std::string& file ) const {
  for( int i = 0; i < _meshes.size(); ++i ) {
    if( strcmp(file.c_str(), _meshes[i].name.c_str()) != 0 ) {
      continue;
    }
    return i;
  }
  return -1;
}

int Scene::findMaterialIndex( const std::string& file ) const {
  for( int i = 0; i < _materials.size(); ++i ) {
    if( strcmp(file.c_str(), _materials[i].name.c_str()) != 0 ) {
      continue;
    }
    return i;
  }
  return -1;
}

void Scene::clean() {
  _parserState = kParserStateWhitespace;
  _objects.clear();
  _textures.clear();
  _meshes.clear();
  _materials.clear();
}
