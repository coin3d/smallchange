#include <SmallChange/nodes/SwitchboardOperator.h>
#include <Inventor/nodes/SoSubNode.h>

#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/events/SoKeyboardEvent.h>

#include <Inventor/errors/SoDebugError.h>

SO_NODE_SOURCE(SwitchboardOperator);

void
SwitchboardOperator::initClass(void)
{
  SO_NODE_INIT_CLASS(SwitchboardOperator, Switchboard, Switchboard);
}

SwitchboardOperator::SwitchboardOperator(void)
{
  this->constructor();
}

SwitchboardOperator::SwitchboardOperator(int numchildren)
: inherited(numchildren)
{
  this->constructor();
}

void
SwitchboardOperator::constructor(void) // private
{
  SO_NODE_CONSTRUCTOR(SwitchboardOperator);

  SO_NODE_ADD_FIELD(key, (UNDEFINED));
  SO_NODE_ADD_FIELD(behavior, (TOGGLE));

  SO_NODE_DEFINE_ENUM_VALUE(Key, ANY);
  SO_NODE_DEFINE_ENUM_VALUE(Key, UNDEFINED);
  SO_NODE_DEFINE_ENUM_VALUE(Key, A);
  SO_NODE_DEFINE_ENUM_VALUE(Key, B);
  SO_NODE_DEFINE_ENUM_VALUE(Key, C);
  SO_NODE_DEFINE_ENUM_VALUE(Key, D);
  SO_NODE_DEFINE_ENUM_VALUE(Key, E);
  SO_NODE_DEFINE_ENUM_VALUE(Key, F);
  SO_NODE_DEFINE_ENUM_VALUE(Key, G);
  SO_NODE_DEFINE_ENUM_VALUE(Key, H);
  SO_NODE_DEFINE_ENUM_VALUE(Key, I);
  SO_NODE_DEFINE_ENUM_VALUE(Key, J);
  SO_NODE_DEFINE_ENUM_VALUE(Key, K);
  SO_NODE_DEFINE_ENUM_VALUE(Key, L);
  SO_NODE_DEFINE_ENUM_VALUE(Key, M);
  SO_NODE_DEFINE_ENUM_VALUE(Key, N);
  SO_NODE_DEFINE_ENUM_VALUE(Key, O);
  SO_NODE_DEFINE_ENUM_VALUE(Key, P);
  SO_NODE_DEFINE_ENUM_VALUE(Key, Q);
  SO_NODE_DEFINE_ENUM_VALUE(Key, R);
  SO_NODE_DEFINE_ENUM_VALUE(Key, S);
  SO_NODE_DEFINE_ENUM_VALUE(Key, T);
  SO_NODE_DEFINE_ENUM_VALUE(Key, U);
  SO_NODE_DEFINE_ENUM_VALUE(Key, V);
  SO_NODE_DEFINE_ENUM_VALUE(Key, W);
  SO_NODE_DEFINE_ENUM_VALUE(Key, X);
  SO_NODE_DEFINE_ENUM_VALUE(Key, Y);
  SO_NODE_DEFINE_ENUM_VALUE(Key, Z);

  SO_NODE_DEFINE_ENUM_VALUE(Behavior, NONE);
  SO_NODE_DEFINE_ENUM_VALUE(Behavior, TOGGLE);
  SO_NODE_DEFINE_ENUM_VALUE(Behavior, HOLD);
  SO_NODE_DEFINE_ENUM_VALUE(Behavior, INVERSE_HOLD);

  SO_NODE_SET_SF_ENUM_TYPE(key, Key);
  SO_NODE_SET_SF_ENUM_TYPE(behavior, Behavior);
}

SwitchboardOperator::~SwitchboardOperator(void) // virtual, protected
{
}

void
SwitchboardOperator::handleEvent(SoHandleEventAction * action)
{
  const SoEvent * ev = action->getEvent();
  if ( ev->isOfType(SoKeyboardEvent::getClassTypeId()) ) {
    const SoKeyboardEvent * event = (const SoKeyboardEvent *) ev;
    SoKeyboardEvent::Key key = event->getKey();
    for ( int idx = 0; idx < this->key.getNum(); idx++ ) {
      if ( this->key[idx] == key ) {
        switch ( idx < this->behavior.getNum() ? this->behavior[idx] : TOGGLE ) {
        case TOGGLE:
          if ( event->getState() == SoKeyboardEvent::DOWN ) {
            if ( idx >= this->enable.getNum() ) this->enable.setNum(idx+1);
            this->enable.set1Value(idx, this->enable[idx] ? FALSE : TRUE);
          }
          break;
        case HOLD:
          if ( idx >= this->enable.getNum() ) this->enable.setNum(idx+1);
          this->enable.set1Value(idx, event->getState() == SoKeyboardEvent::DOWN ? TRUE : FALSE);
          break;
        case INVERSE_HOLD:
          if ( idx >= this->enable.getNum() ) this->enable.setNum(idx+1);
          this->enable.set1Value(idx, event->getState() == SoKeyboardEvent::DOWN ? FALSE : TRUE);
          break;
        default:
          break;
        }
      }
    }
  }
}

