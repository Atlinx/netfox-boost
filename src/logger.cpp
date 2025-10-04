#include "logger.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/templates/local_vector.hpp>

_NetfoxLogger::LogLevel _NetfoxLogger::log_level;
HashMap<String, _NetfoxLogger::LogLevel> _NetfoxLogger::module_log_level;
HashMap<int, Vector<Callable>> _NetfoxLogger::_tags;
Vector<Callable> _NetfoxLogger::_ordered_tags;

void _NetfoxLogger::set_module_log_level(const Dictionary& p_log_levels) {
	module_log_level.clear();
	for (auto key : p_log_levels.keys()) {
		module_log_level[key] = (LogLevel) (int) p_log_levels[key];
	}
}

Dictionary _NetfoxLogger::get_module_log_level() {
	Dictionary dict;
	for (auto key_value : module_log_level) {
		dict[key_value.key] = key_value.value;
	}
	return dict;
}

Ref<_NetfoxLogger> _NetfoxLogger::new_(String p_module, String p_name)
{
	Ref<_NetfoxLogger> logger;
	logger.instantiate();
	logger->module = p_module;
	logger->name = p_name;
	return logger;
}

Ref<_NetfoxLogger> _NetfoxLogger::for_netfox(String p_name)
{
	return _NetfoxLogger::new_("netfox", p_name);
}

Ref<_NetfoxLogger> _NetfoxLogger::for_noray(String p_name)
{
	return _NetfoxLogger::new_("netfox.noray", p_name);
}
Ref<_NetfoxLogger> _NetfoxLogger::for_extras(String p_name)
{
	return _NetfoxLogger::new_("netfox.extras", p_name);
}

Dictionary _NetfoxLogger::make_setting(String name)
{
	Dictionary dict;

	dict["name"] = name;
	dict["value"] = DEFAULT_LOG_LEVEL;
	dict["type"] = Variant::INT;
	dict["hint"] = PROPERTY_HINT_ENUM;
	dict["hint_string"] = "All,Trace,Debug,Info,Warning,Error,None";

	return dict;
}

void _NetfoxLogger::register_tag(Callable tag, int priority)
{
	if (!_tags.has(priority)) {
		_tags[priority] = Vector<Callable>();
	}
	_tags[priority].push_back(tag);

	_ordered_tags.clear();
	
	LocalVector<int> prio_groups;
	for (const KeyValue<int, Vector<Callable>>& E : _tags) {
		prio_groups.push_back(E.key);
	}
	prio_groups.sort();

	for (int prio_group : prio_groups) {
		const Vector<Callable>& tag_group = _tags[prio_group];
		for (auto tag : tag_group) {
			_ordered_tags.push_back(tag);
		}
	}
}

void _NetfoxLogger::free_tag(Callable tag)
{
	LocalVector<int> keys_to_erase;
	for (KeyValue<int, Vector<Callable>>& E : _tags) {
		Vector<Callable>& priority_group = E.value;
		priority_group.erase(tag);

		if (priority_group.is_empty()) {
			keys_to_erase.push_back(E.key);
		}
	}

	for (int key : keys_to_erase) {
		_tags.erase(key);
	}

	_ordered_tags.erase(tag);
}

bool _NetfoxLogger::_check_log_level(LogLevel level)
{
	LogLevel cmp_level = log_level;
	if (level < cmp_level) {
		return false;
	}

	if (module_log_level.has(module)) {
		LogLevel module_level = module_log_level[module];
		return level >= module_level;
	}

	return true;
}

String _NetfoxLogger::_format_text(String text, Array values, LogLevel level)
{
	level = CLAMP(level, LOG_MIN, LOG_MAX);

	PackedStringArray result;

	result.append(vformat("[%s]", level_prefixes[level]));
	
	for (Callable tag : _ordered_tags)
		result.append(vformat("[%s]", tag.call()));
	result.append(vformat("[%s::%s] ", module, name));

	if (values.is_empty()) {
		result.append(text);
	} else {
		result.append(text.format(values));
	}

	return String("").join(result);
}

void _NetfoxLogger::_log_text(String text, Array values, LogLevel level)
{
	if(_check_log_level(level))
	{
		UtilityFunctions::print(_format_text(text, values, level));
	}
}

void _NetfoxLogger::trace(String text, Array values)
{
	_log_text(text, values, LOG_TRACE);
}

void _NetfoxLogger::debug(String text, Array values)
{
	_log_text(text, values, LOG_DEBUG);
}

void _NetfoxLogger::info(String text, Array values)
{
	_log_text(text, values, LOG_INFO);
}

void _NetfoxLogger::warning(String text, Array values)
{
	if(_check_log_level(LOG_WARN))
	{
		String formatted_text = _format_text(text, values, LOG_WARN);
		UtilityFunctions::push_warning(formatted_text);

		UtilityFunctions::print(formatted_text);
	}
}

void _NetfoxLogger::error(String text, Array values)
{
	if(_check_log_level(LOG_ERROR))
	{
		String formatted_text = _format_text(text, values, LOG_ERROR);
		UtilityFunctions::push_error(formatted_text);

		UtilityFunctions::print(formatted_text);
	}
}

void _NetfoxLogger::_bind_methods() 
{
	log_level = (LogLevel) (int) ProjectSettings::get_singleton()->get_setting("netfox/logging/log_level", DEFAULT_LOG_LEVEL);

	module_log_level["netfox"] = (LogLevel) (int) ProjectSettings::get_singleton()->get_setting("netfox/logging/netfox_log_level", DEFAULT_LOG_LEVEL);
	module_log_level["netfox.noray"] = (LogLevel) (int) ProjectSettings::get_singleton()->get_setting("netfox/logging/netfox_noray_log_level", DEFAULT_LOG_LEVEL);
	module_log_level["netfox.extras"] = (LogLevel) (int) ProjectSettings::get_singleton()->get_setting("netfox/logging/netfox_extras_log_level", DEFAULT_LOG_LEVEL);

	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("new_", "p_module", "p_name"), &_NetfoxLogger::new_);

	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("set_log_level", "level"), &_NetfoxLogger::set_log_level);
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("get_log_level"), &_NetfoxLogger::get_log_level);

	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("set_module_log_level", "log_levels"), &_NetfoxLogger::set_module_log_level);
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("get_module_log_level"), &_NetfoxLogger::get_module_log_level);

	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("for_netfox", "p_name"), &_NetfoxLogger::for_netfox);
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("for_noray", "p_name"), &_NetfoxLogger::for_noray);
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("for_extras", "p_name"), &_NetfoxLogger::for_extras);
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("make_setting", "name"), &_NetfoxLogger::make_setting);
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("register_tag", "tag", "priority"), &_NetfoxLogger::register_tag, DEFVAL(0));
	ClassDB::bind_static_method("_NetfoxLogger", D_METHOD("free_tag", "tag"), &_NetfoxLogger::free_tag);
	ClassDB::bind_method(D_METHOD("trace", "text", "values"), &_NetfoxLogger::trace, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("debug", "text", "values"), &_NetfoxLogger::debug, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("info", "text", "values"), &_NetfoxLogger::info, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("warning", "text", "values"), &_NetfoxLogger::warning, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("error", "text", "values"), &_NetfoxLogger::error, DEFVAL(Array()));

	ADD_PROPERTY(
		PropertyInfo(Variant::INT, "log_level", PROPERTY_HINT_ENUM, "Trace,Debug,Info,Warn,Error"),
		"set_log_level",
		"get_log_level"
	);

	ADD_PROPERTY(
		PropertyInfo(Variant::DICTIONARY, "module_log_level"),
		"set_module_log_level",
		"get_module_log_level"
	);

	BIND_ENUM_CONSTANT(LOG_MIN);
	BIND_ENUM_CONSTANT(LOG_TRACE);
	BIND_ENUM_CONSTANT(LOG_DEBUG);
	BIND_ENUM_CONSTANT(LOG_INFO);
	BIND_ENUM_CONSTANT(LOG_WARN);
	BIND_ENUM_CONSTANT(LOG_ERROR);
	BIND_ENUM_CONSTANT(LOG_MAX);
}