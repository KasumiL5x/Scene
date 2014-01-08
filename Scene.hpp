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

#ifndef __Scene__
#define __Scene__

#include <string>
#include <vector>

class Scene {
private:
  enum ParserState : unsigned int {
    kParserStateWhitespace,
    kParserStateScene,
    kParserStateResources,
    kParserStateResourceTexture,
    kParserStateResourceMesh,
    kParserStateResourceMaterial,
    kParserStateObjects,
    kParserStateObjectsObj,
    kParserStateLights,
    kParserStateLightsLight
  };

public:
  struct Vector {
    float x;
    float y;
    float z;

    Vector()
      : x(0.0f), y(0.0f), z(0.0f) {
    }
    Vector( float valX, float valY, float valZ )
      : x(valX), y(valY), z(valZ) {
    }
    Vector( float val )
      : x(val), y(val), z(val) {
    }
  };

  struct Texture {
    std::string file;
    std::string name;

    Texture()
      : file(""), name("") {
    }

    void reset() {
      file = "";
      name = "";
    }
  };

  struct Mesh {
    std::string file;
    std::string name;

    Mesh()
      : file(""), name("") {
    }

    void reset() {
      file = "";
      name = "";
    }
  };

  struct Material {
    std::string name;
    Vector      color;
    float       specSize;
    int         diffuseTex;
    int         normalTex;

    Material()
      : name(""), color(1.0f), specSize(0.0f), diffuseTex(-1), normalTex(-1) {
    }

    void reset() {
      name       = "";
      color      = Vector(1.0f);
      specSize   = 0.0f;
      diffuseTex = -1;
      normalTex  = -1;
    }
  };

  struct Object {
    std::string name;
    Vector      position;
    Vector      orientation;
    Vector      scale;
    int         mesh;
    int         material;

    Object()
      : name(""), position(0.0f), orientation(0.0f), scale(1.0f), mesh(-1), material(-1) {
    }

    void reset() {
      name        = "";
      position    = Vector(0.0f);
      orientation = Vector(0.0f);
      scale       = Vector(1.0f);
      mesh        = -1;
      material    = -1;
    }
  };

  enum LightType : unsigned int {
    kLightTypePoint,
    kLightTypeSpot,
    kLightTypeDirectional
  };

  struct Light {
    LightType type;
    Vector    diffuseColor;
    float     diffuseIntensity;
    Vector    specularColor;
    float     specularIntensity;
    Vector    position;
    float     range;
    Vector    direction;
    bool      shadows;
    float     shadowBias;
    float     coneInnerAngle;
    float     coneOuterAngle;

    Light() {
    }

    void reset() {
      type              = kLightTypePoint;
      diffuseColor      = Vector(1.0f);
      diffuseIntensity  = 1.0f;
      specularColor     = Vector(1.0f);
      specularIntensity = 1.0f;
      position          = Vector(0.0f);
      range             = 64.0f;
      direction         = Vector(0.0f);
      shadows           = true;
      shadowBias        = 0.00001f;
      coneInnerAngle    = 10.0f;
      coneOuterAngle    = 12.0f;
    }
  };

public:
  Scene();
  ~Scene();

  bool                         load         ( const std::string& file );
  void                         debugOutput  () const;
  const std::vector<Object>&   objects      () const;
  const std::vector<Texture>&  textures     () const;
  const std::vector<Mesh>&     meshes       () const;
  const std::vector<Material>& materials    () const;
  const std::vector<Light>&    lights       () const;
  unsigned int                 objectCount  () const;
  unsigned int                 textureCount () const;
  unsigned int                 meshCount    () const;
  unsigned int                 materialCount() const;
  unsigned int                 lightCount   () const;

private:
  void parseLine        ( const std::string& line, Object& obj, Texture& tex, Mesh& mesh, Material& mat, Light& light );
  void readVector       ( const std::string& line, Vector* outVec ) const;
  void parseBool        ( const std::string& value, bool* out );
  int  findTextureIndex ( const std::string& file ) const;
  int  findMeshIndex    ( const std::string& file ) const;
  int  findMaterialIndex( const std::string& file ) const;
  void clean            ();

private:
  ParserState           _parserState;
  std::vector<Object>   _objects;
  std::vector<Texture>  _textures;
  std::vector<Mesh>     _meshes;
  std::vector<Material> _materials;
  std::vector<Light>    _lights;
};

#endif /* __Scene__ */
