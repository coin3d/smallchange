#ifndef COIN_SWITCHBOARDOPERATOR_H
#define COIN_SWITCHBOARDOPERATOR_H

/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *  
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use Coin with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

#include <SmallChange/nodes/Switchboard.h>
#include <Inventor/fields/SoMFEnum.h>
#include <Inventor/events/SoKeyboardEvent.h>

class SwitchboardOperator : public Switchboard {
  typedef Switchboard inherited;
  SO_NODE_HEADER(SwitchboardOperator);

public:
  static void initClass(void);
  SwitchboardOperator(void);
  SwitchboardOperator(int numchildren);

  enum Behavior {
    NONE, TOGGLE, HOLD, INVERSE_HOLD
  };

  enum Key {
    ANY = SoKeyboardEvent::ANY,
    UNDEFINED = SoKeyboardEvent::UNDEFINED,
    A = SoKeyboardEvent::A,
    B = SoKeyboardEvent::B,
    C = SoKeyboardEvent::C,
    D = SoKeyboardEvent::D,
    E = SoKeyboardEvent::E,
    F = SoKeyboardEvent::F,
    G = SoKeyboardEvent::G,
    H = SoKeyboardEvent::H,
    I = SoKeyboardEvent::I,
    J = SoKeyboardEvent::J,
    K = SoKeyboardEvent::K,
    L = SoKeyboardEvent::L,
    M = SoKeyboardEvent::M,
    N = SoKeyboardEvent::N,
    O = SoKeyboardEvent::O,
    P = SoKeyboardEvent::P,
    Q = SoKeyboardEvent::Q,
    R = SoKeyboardEvent::R,
    S = SoKeyboardEvent::S,
    T = SoKeyboardEvent::T,
    U = SoKeyboardEvent::U,
    V = SoKeyboardEvent::V,
    W = SoKeyboardEvent::W,
    X = SoKeyboardEvent::X,
    Y = SoKeyboardEvent::Y,
    Z = SoKeyboardEvent::Z,
    SPACE = SoKeyboardEvent::SPACE
  };

  SoMFEnum key;
  SoMFEnum behavior;

  virtual void handleEvent(SoHandleEventAction * action);

protected:
  virtual ~SwitchboardOperator(void);

private:
  void constructor(void);

};

#endif // !COIN_SWITCHBOARDOPERATOR_H
