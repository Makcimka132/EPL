/*
This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <llvm-c/Core.h>
#include "Node.hpp"
#include "../Types.hpp"

class NodeContinue : public Node {
public:
    int loc;

    NodeContinue(int loc);
    int getWhileLoop();
    RaveValue generate() override;
    Type* getType() override;
    
    Node* comptime() override;
    Node* copy() override;
    void check() override;
};