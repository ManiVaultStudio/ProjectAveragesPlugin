#pragma once

#include "actions/GroupAction.h"
#include "actions/DecimalAction.h"
#include "actions/IntegralAction.h"
#include "actions/StringAction.h"
#include "actions/TriggerAction.h"

/** All GUI related classes are in the ManiVault Graphical User Interface namespace */
using namespace mv::gui;

class SettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    SettingsAction(QObject* parent = nullptr);

public: // Action getters

   

public:
    
};
