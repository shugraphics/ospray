// Copyright 2009-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "Builder.h"
#include "ospray_testing.h"

using namespace ospcommon::math;

namespace ospray {
namespace testing {

struct SubdivisionCube : public detail::Builder
{
  SubdivisionCube() = default;
  ~SubdivisionCube() override = default;

  cpp::Group buildGroup() const override;
};

// Inlined definitions ////////////////////////////////////////////////////

cpp::Group SubdivisionCube::buildGroup() const
{
  // vertices of a cube
  std::vector<vec3f> vertices = {{-1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
      {-1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, 1.0f},
      {-1.0f, 1.0f, 1.0f}};

  // color per vertex
  std::vector<vec4f> colors = {{0.0f, 0.0f, 0.0f, 1.f},
      {1.0f, 0.0f, 0.0f, 1.f},
      {1.0f, 0.0f, 1.0f, 1.f},
      {0.0f, 0.0f, 1.0f, 1.f},
      {0.0f, 1.0f, 0.0f, 1.f},
      {1.0f, 1.0f, 0.0f, 1.f},
      {1.0f, 1.0f, 1.0f, 1.f},
      {0.0f, 1.0f, 1.0f, 1.f}};

  // number of vertex indices per face
  std::vector<unsigned int> faces = {4, 4, 4, 4, 4, 4};

  // vertex indices for all faces
  std::vector<unsigned int> indices = {
      0,
      4,
      5,
      1,
      1,
      5,
      6,
      2,
      2,
      6,
      7,
      3,
      0,
      3,
      7,
      4,
      4,
      7,
      6,
      5,
      0,
      1,
      2,
      3,
  };

  // vertex crease indices and weights
  std::vector<unsigned int> vertexCreaseIndices = {0, 1, 2, 3, 4, 5, 6, 7};
  std::vector<float> vertexCreaseWeights = {
      2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f};

  // edge crease indices and weights
  std::vector<vec2i> edgeCreaseIndices = {{0, 1},
      {1, 2},
      {2, 3},
      {3, 0},
      {4, 5},
      {5, 6},
      {6, 7},
      {7, 4},
      {0, 4},
      {1, 5},
      {2, 6},
      {3, 7}};
  std::vector<float> edgeCreaseWeights = {
      2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f};

  // global level of tessellation
  float level = 5.f;

  // create the OSPRay geometry and set all parameters
  cpp::Geometry geometry("subdivision");

  geometry.setParam("vertex.position", cpp::Data(vertices));
  geometry.setParam("vertex.color", cpp::Data(colors));
  geometry.setParam("face", cpp::Data(faces));
  geometry.setParam("index", cpp::Data(indices));
  geometry.setParam("vertexCrease.index", cpp::Data(vertexCreaseIndices));
  geometry.setParam("vertexCrease.weight", cpp::Data(vertexCreaseWeights));
  geometry.setParam("edgeCrease.index", cpp::Data(edgeCreaseIndices));
  geometry.setParam("edgeCrease.weight", cpp::Data(edgeCreaseWeights));
  geometry.setParam("level", level);
  geometry.setParam("mode", (int)OSP_SUBDIVISION_PIN_CORNERS);

  geometry.commit();

  cpp::GeometricModel model(geometry);

  if (rendererType == "pathtracer" || rendererType == "scivis") {
    cpp::Material material(rendererType, "obj");
    material.commit();
    model.setParam("material", material);
  }

  model.commit();

  cpp::Group group;

  group.setParam("geometry", cpp::Data(model));
  group.commit();

  return group;
}

OSP_REGISTER_TESTING_BUILDER(SubdivisionCube, subdivision_cube);

} // namespace testing
} // namespace ospray
