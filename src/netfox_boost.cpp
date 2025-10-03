#include "netfox_boost.h"

void NetfoxBoost::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &NetfoxBoost::print_type);
}

void NetfoxBoost::print_type(const Variant &p_variant) const {
	print_line(vformat("Type: %d", p_variant.get_type()));
}
