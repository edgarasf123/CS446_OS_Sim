#ifndef _CONFIG_MANAGER
#define _CONFIG_MANAGER

#include <unordered_map>
#include <vector>

#include <string>

/**
 * @brief Enumeration for config option value type.
 * 
 */
enum class ConfigType
{
	Int, Double, String
};

/**
 * @brief Data structure used by ConfigManager 
 * 		  to storing individual configuration options.
 * 
 */
struct ConfigOption
{
	ConfigType type;
	bool initialized;
	
	long int value_i;
	double value_d;
	std::string value_s;
};

class ConfigManager
{
	private:
		std::unordered_map<std::string, ConfigOption> configOptions;

	public:
		/**
		 * @brief Constructor for ConfigManager.
		 */
		ConfigManager();
		/**
		 * @brief Destructor for ConfigManager.
		 */
		~ConfigManager();

		/**
		 * @brief Adds config option to config manager.
		 * @details Checks if config option already exist,
		 * 			if not then it adds new option to config
		 * 			manager.
		 * 
		 * @param label Config option label.
		 * @param type Config option type.
		 * 
		 * @return True if config option created successfully.
		 */
		bool AddOption( std::string label, ConfigType type );

		/**
		 * @brief Removes option from config manager.
		 * 
		 * @param label Config option label.
		 */
		void RemoveOption( std::string label );

		/**
		 * @brief Checks whether config option exist or not in config manager.
		 * 
		 * @param label Config option label.
		 * @return True if config option exist.
		 */
		bool OptionExist( std::string label );

		/**
		 * @brief Checks whether value has been set for config option.
		 * 
		 * @param label Config option label.
		 * @return True if config option is initialized.
		 */
		bool OptionInitialized( std::string label );

		/**
		 * @brief Returns config option type for specified config option.
		 * 
		 * @param label Config option label.
		 * @return Config option type of config option.
		 */
		ConfigType OptionType( std::string label );

		/**
		 * @brief Sets config option with value in string format.
		 * @details Sets config option with value in string format. Automatically
		 * 			converts the string into specific type according to config 
		 * 			option type.
		 * 
		 * @param label Config option label.
		 * @param strVal Value to set in string format.
		 */
		void SetStr( std::string label, std::string strVal );

		/**
		 * @brief Sets integer config option with the value.
		 * 
		 * @param label Config option label.
		 * @param int Value to set.
		 */
		void SetInt( std::string label, long int val );
		
		/**
		 * @brief Sets double config option with the value.
		 * 
		 * @param label Config option label.
		 * @param int Value to set.
		 */
		void SetDouble( std::string label, double val );
		
		/**
		 * @brief Sets string config option with the value.
		 * 
		 * @param label Config option label.
		 * @param int Value to set.
		 */
		void Set( std::string label, std::string val );

		/**
		 * @brief Returns pointer to config option data.
		 * 
		 * @param label Config option label.
		 * @return Pointer to config option.
		 */
		ConfigOption * GetOption( std::string label );

		/**
		 * @brief Populates labels vector with all config option labels.
		 * 
		 * @param labels string vector to populate with labels.
		 */
		void GetOptionLabels( std::vector<std::string> &labels );

		/**
		 * @brief Returns value of integet config option.
		 * 
		 * @param label Config option label.
		 * @return Value of config option.
		 */
		long int GetInt( std::string label );
		/**
		 * @brief Returns value of integet config option.
		 * 
		 * @param label Config option label.
		 * @return Value of config option.
		 */
		double GetDouble( std::string label );
		/**
		 * @brief Returns value of integet config option.
		 * @details Returns value of integet config option. Unlike GetInt and GetDouble,
		 * 			GetStr doesn't require for config option type to be string. It will
		 * 			return integer and double types in string format.
		 * 
		 * @param label Config option label.
		 * @return Value of config option.
		 */
		std::string GetStr( std::string label );
};

#endif // _CONFIG_MANAGER