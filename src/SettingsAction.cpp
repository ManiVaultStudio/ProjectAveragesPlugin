#include "SettingsAction.h"

SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, "SettingsAction", true)
    
{
    setText("Settings");

}
