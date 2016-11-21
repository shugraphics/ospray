// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#undef NDEBUG

#include "PathTracer.h"
// ospray
#include "common/Data.h"
#include "lights/Light.h"
#include "geometry/Instance.h"
// ispc exports
#include "PathTracer_ispc.h"
#include "Material_ispc.h"
#include "GeometryLight_ispc.h"
// std
#include <map>

namespace ospray {
  PathTracer::PathTracer() : Renderer(), geometryLights(0)
  {
    ispcEquivalent = ispc::PathTracer_create(this);
  }

  PathTracer::~PathTracer()
  {
    destroyGeometryLights();
  }

  /*! \brief create a material of given type */
  Material *PathTracer::createMaterial(const char *type)
  {
    std::string ptType = std::string("PathTracer_")+type;
    Material *material = Material::createMaterial(ptType.c_str());
    if (!material) {
      std::map<std::string,int> numOccurrances;
      const std::string T = type;
      if (numOccurrances[T] == 0)
        std::cout << "#osp:PT: does not know material type '" << type << "'" <<
          " (replacing with OBJMaterial)" << std::endl;
      numOccurrances[T] ++;
      material = Material::createMaterial("PathTracer_OBJMaterial");
      // throw std::runtime_error("invalid path tracer material "+std::string(type));
    }
    material->refInc();
    return material;
  }


  void PathTracer::generateGeometryLights(const Model * const model
      , const affine3f& xfm
      )
  {
    for(auto &geo : model->geometry) {
      // recurse instances
      const Ref<Instance> inst = geo.dynamicCast<Instance>();
      if (inst) {
        const affine3f instXfm = xfm * inst->xfm;
        generateGeometryLights(inst->instancedScene.ptr, instXfm);
      } else
        if (geo->material && geo->material->getIE()
            && ispc::PathTraceMaterial_isEmissive(geo->material->getIE()))
          lightArray.push_back(ispc::GeometryLight_create(geo->getIE(),
                (const ispc::AffineSpace3f&)xfm));
    }
  }

  void PathTracer::destroyGeometryLights()
  {
    for (size_t i = 0; i < geometryLights; i++)
      ispc::delete_uniform(lightArray[i]);
  }

  void PathTracer::commit()
  {
    Renderer::commit();

    destroyGeometryLights();
    lightArray.clear();

    if (model) {
      generateGeometryLights(model, affine3f(one));
      geometryLights = lightArray.size();
    }

    lightData = (Data*)getParamData("lights");
    if (lightData) {
      for (uint32_t i = 0; i < lightData->size(); i++)
        lightArray.push_back(((Light**)lightData->data)[i]->getIE());
    }

    void **lightPtr = lightArray.empty() ? NULL : &lightArray[0];

    const int32 maxDepth = getParam1i("maxDepth", 20);
    const float minContribution = getParam1f("minContribution", 0.01f);
    const float maxRadiance = getParam1f("maxContribution", getParam1f("maxRadiance", inf));
    Texture2D *backplate = (Texture2D*)getParamObject("backplate", NULL);

    ispc::PathTracer_set(getIE(), maxDepth, minContribution, maxRadiance,
                         backplate ? backplate->getIE() : NULL,
                         lightPtr, lightArray.size());
  }

  OSP_REGISTER_RENDERER(PathTracer,pathtracer);
  OSP_REGISTER_RENDERER(PathTracer,pt);

  extern "C" void ospray_init_module_pathtracer()
  {
    printf("Loaded plugin 'pathtracer' ...\n");
  }

}

