# Register Addresses
DVC_STATUS = 0x0  # Address in decimal: 0
DVC_MEASUREMENT_CONFIG = 0x1  # Address in decimal: 1
DVC_PROBE_CONFIG = 0x2  # Address in decimal: 2
DVC_SAMPLE_DATA = 0x64  # Address in decimal: 100

# Bitfields
# DVC_STATUS
DVC_STATUS_POWER_GOOD_POSITION = 0  # Bit position
DVC_STATUS_POWER_GOOD_LENGTH = 1  # Bit length
DVC_STATUS_USB_CONNECTED_POSITION = 1  # Bit position
DVC_STATUS_USB_CONNECTED_LENGTH = 1  # Bit length

# DVC_MEASUREMENT_CONFIG
DVC_MEASUREMENT_CONFIG_START_MEASURE_POSITION = 0  # Bit position
DVC_MEASUREMENT_CONFIG_START_MEASURE_LENGTH = 1  # Bit length
DVC_MEASUREMENT_CONFIG_STOP_MEASURE_POSITION = 1  # Bit position
DVC_MEASUREMENT_CONFIG_STOP_MEASURE_LENGTH = 1  # Bit length
DVC_MEASUREMENT_CONFIG_MEASURE_INTERVAL_POSITION = 2  # Bit position
DVC_MEASUREMENT_CONFIG_MEASURE_INTERVAL_LENGTH = 2  # Bit length

# DVC_PROBE_CONFIG
DVC_PROBE_CONFIG_USED_PROBES_POSITION = 0  # Bit position
DVC_PROBE_CONFIG_USED_PROBES_LENGTH = 4  # Bit length
DVC_PROBE_CONFIG_PROBE_1_CONFIG_POSITION = 4  # Bit position
DVC_PROBE_CONFIG_PROBE_1_CONFIG_LENGTH = 5  # Bit length
DVC_PROBE_CONFIG_PROBE_2_CONFIG_POSITION = 9  # Bit position
DVC_PROBE_CONFIG_PROBE_2_CONFIG_LENGTH = 5  # Bit length
DVC_PROBE_CONFIG_PROBE_3_CONFIG_POSITION = 14  # Bit position
DVC_PROBE_CONFIG_PROBE_3_CONFIG_LENGTH = 5  # Bit length
DVC_PROBE_CONFIG_PROBE_4_CONFIG_POSITION = 19  # Bit position
DVC_PROBE_CONFIG_PROBE_4_CONFIG_LENGTH = 5  # Bit length

# DVC_SAMPLE_DATA
DVC_SAMPLE_DATA_SAMPLE_1_POSITION = 0  # Bit position
DVC_SAMPLE_DATA_SAMPLE_1_LENGTH = 12  # Bit length
DVC_SAMPLE_DATA_SAMPLE_2_POSITION = 12  # Bit position
DVC_SAMPLE_DATA_SAMPLE_2_LENGTH = 12  # Bit length