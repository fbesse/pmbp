//
//  message.cc
//  fpmbp2
//
//  Created by Frederic Besse on 20/11/2012.
//  Copyright (c) 2012 Frederic Besse. All rights reserved.
//

//------------------------------------------------------------------------------

#include "message.h"
#include "node.h"
#include <sstream>

//------------------------------------------------------------------------------

namespace pmbp {
  
//------------------------------------------------------------------------------

Message::Message() : node(0)
{
}
  
//------------------------------------------------------------------------------
  
Message::Message(Node* n) : node(n)
{
  values.resize(node->Size());
}
  
//------------------------------------------------------------------------------

void Message::Set(int k, float value)
{
  values[k] = value;
}

//------------------------------------------------------------------------------
  
float Message::GetValue(int k) const
{
  return values[k];
}
  
//------------------------------------------------------------------------------

void Message::SetUniform()
{
  for(int i=0; i<values.size(); ++i){
    values[i] = 0.f;
  }
}
  
//------------------------------------------------------------------------------

void Message::Normalize()
{
  float sum = 0;
  
  for(int k=0; k<values.size(); ++k){
    sum += values[k];
  }

  if(sum != 0){
    float constant = 0.f;
    float rest = (sum-constant)/values.size();
    
    for(int k=0; k<values.size(); ++k){
      values[k] -= rest;
    }
  }
}
  
//------------------------------------------------------------------------------
  
std::string Message::Summary() const
{
  std::stringstream summary;
  for(int i=0; i<values.size(); ++i){
    summary << "[a=" << values[i] << "]";
  }
  
  return summary.str();
}
  
//------------------------------------------------------------------------------
  
}

//------------------------------------------------------------------------------