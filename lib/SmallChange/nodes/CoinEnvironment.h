#ifndef COIN_COINENVIRONMENT_H
#define COIN_COINENVIRONMENT_H
/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFColor.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/elements/SoEnvironmentElement.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API CoinEnvironment : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(CoinEnvironment);

public:
  static void initClass(void);
  CoinEnvironment(void);

  enum FogType {
    NONE = SoEnvironmentElement::NONE,
    HAZE = SoEnvironmentElement::HAZE,
    FOG = SoEnvironmentElement::FOG,
    SMOKE = SoEnvironmentElement::SMOKE
  };

  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);

  SoSFFloat ambientIntensity;
  SoSFColor ambientColor;
  SoSFVec3f attenuation;
  SoSFEnum fogType;
  SoSFColor fogColor;
  SoSFFloat fogVisibility;
  SoSFFloat fogStart;

protected:
  virtual ~CoinEnvironment();
};

#endif // !COIN_COINENVIRONMENT_H
