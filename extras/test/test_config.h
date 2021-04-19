/* common configuration */
#define CONFIG_CRC16_ENABLED
#define CONFIG_CRC8_ALGO CRC8_TAB_16LH
#define CONFIG_CRC16_ALGO CRC16_TAB_16LH

#if defined(T03)
# define CONFIG_MAX_SRCH_FILTERS 5
#else
# define CONFIG_MAX_SRCH_FILTERS 10
#endif
