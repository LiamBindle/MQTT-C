#ifndef __MQTT_RULES_H__
#define __MQTT_RULES_H__

#define MQTT_BITFIELD_RULE_VIOLOATION(bitfield, rule_value, rule_mask) ((bitfield ^ rule_value) & rule_mask)

#endif