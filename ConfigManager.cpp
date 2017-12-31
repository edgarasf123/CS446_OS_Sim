#include "ConfigManager.h"

#include <stdexcept>

using std::string;
using std::to_string;
using std::vector;
using std::runtime_error;
using std::invalid_argument;

ConfigManager::ConfigManager(){
    
}
ConfigManager::~ConfigManager(){
    configOptions.clear();
}

bool ConfigManager::AddOption( string label, ConfigType type )
{
    ConfigOption * option = GetOption(label);
    if( option != NULL)
        return false;

    ConfigOption newOption;
    newOption.type = type;
    newOption.initialized = false;
    configOptions.insert({label, newOption});

    return true;
}
void ConfigManager::RemoveOption( string label )
{
    configOptions.erase( label );
}

bool ConfigManager::OptionExist( string label )
{
    ConfigOption * option = GetOption(label);
    return option != NULL;
}

bool ConfigManager::OptionInitialized( string label )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error("ConfigManager::OptionInitialized Attempt to access invalid config option (" + label + ")");
    return option->initialized;
}

ConfigType ConfigManager::OptionType( string label )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::OptionType Attempt to access invalid config option (" + label + ")" );
    return option->type;
}



void ConfigManager::SetStr( string label, string strVal )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::SetStr Attempt to access invalid config option (" + label + ")" );
    switch(option->type)
    {
        case ConfigType::Int:
            {
                char* endptr = 0;
                long int v = strtol(strVal.c_str(), &endptr, 10);

                if(*endptr != '\0' || endptr == strVal)
                    throw invalid_argument( "ConfigManager::SetStr Attempt to set invalid int for config option (" + label + ")" );
                option->value_s = strVal;
                option->value_i = v;
                option->initialized = true;
            } break;
        case ConfigType::Double:
            {
                char* endptr = 0;
                double v = strtod(strVal.c_str(), &endptr);

                if(*endptr != '\0' || endptr == strVal)
                    throw invalid_argument( "ConfigManager::SetStr Attempt to set invalid double for config option (" + label + ")" );
                option->value_s = strVal;
                option->value_d = v;
                option->initialized = true;
            } break;
        case ConfigType::String:
            {
                option->value_s = strVal;
                option->initialized = true;
            }
    }
}
void ConfigManager::SetInt( string label, long int val )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::Set Attempt to access invalid config option (" + label + ")" );
    if( option->type != ConfigType::Int )
        throw runtime_error( "ConfigManager::Set Attempt to modify config option with incorrect type (" + label + ")" );
    option->value_s = to_string(val);
    option->value_i = val;
    option->initialized = true;
}
void ConfigManager::SetDouble( string label, double val )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::Set Attempt to access invalid config option (" + label + ")" );
    if( option->type != ConfigType::Double )
        throw runtime_error( "ConfigManager::Set Attempt to modify config option with incorrect type (" + label + ")" );
    
    option->value_s = to_string(val);
    option->value_d = val;
    option->initialized = true;
}
void ConfigManager::Set( string label, string val )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::Set Attempt to access invalid config option (" + label + ")" );
    if( option->type != ConfigType::String )
        throw runtime_error( "ConfigManager::Set Attempt to modify config option with incorrect type (" + label + ")" );
    
    option->value_s = val;
    option->initialized = true;
}
ConfigOption * ConfigManager::GetOption( string label )
{
    auto search = configOptions.find(label);
    if(search != configOptions.end()) {
        return &(search->second);
    }
    return NULL;
}
void ConfigManager::GetOptionLabels( vector<string> &labels )
{
    labels.clear();
    for ( auto it = configOptions.begin(); it != configOptions.end(); ++it )
        labels.push_back( it->first );
}

long int ConfigManager::GetInt( string label )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::GetInt Attempt to access invalid config option (" + label + ")" );
    if( option->type != ConfigType::Int )
        throw runtime_error( "ConfigManager::GetInt Attempt to access config option with incorrect type (" + label + ")" );
    if( !option->initialized )
        throw runtime_error( "ConfigManager::GetInt Attempt to access uninitialized config option (" + label + ")" );
    
    return option->value_i;
};

double ConfigManager::GetDouble( string label )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error( "ConfigManager::GetDouble Attempt to access invalid config option (" + label + ")" );
    if( option->type != ConfigType::Double )
        throw runtime_error( "ConfigManager::GetDouble Attempt to access config option with incorrect type (" + label + ")" );
    if( !option->initialized )
        throw runtime_error( "ConfigManager::GetDouble Attempt to access uninitialized config option (" + label + ")" );
    
    return option->value_d;
}

string ConfigManager::GetStr( string label )
{
    ConfigOption * option = GetOption(label);
    if( option == NULL)
        throw runtime_error("ConfigManager::GetStr Attempt to access invalid config option (" + label + ")" );
    if( !option->initialized )
        throw runtime_error("ConfigManager::GetStr Attempt to access uninitialized config option (" + label + ")" );
    
    return option->value_s;
}