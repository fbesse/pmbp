//
//  message.h
//  fpmbp2
//
//  Created by Frederic Besse on 20/11/2012.
//  Copyright (c) 2012 Frederic Besse. All rights reserved.
//

#ifndef fpmbp2_message_h
#define fpmbp2_message_h

//------------------------------------------------------------------------------

#include <vector>
#include "state.h"

//------------------------------------------------------------------------------

namespace pmbp {

//------------------------------------------------------------------------------
  
class Node;
  
//------------------------------------------------------------------------------

class Message{
public:
  Message();
  Message(Node* n);
  
  void Set(int k, float value);
  float GetValue(int k) const;
  void SetUniform();
  void Normalize();
  
  std::string Summary() const;
  
private:
  Node* node;
  std::vector<float> values;
};

//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------

#endif
