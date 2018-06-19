#ifndef UTILITY_H__
#define UTILITY_H__

#define log_to(where, msg, args...)			\
  do {									\
  fprintf(where, msg "\n", ##args);		\
  } while (0)

#define log_critical(msg, args...)					\
  do {									\
  log_to(stderr, "[CRIT-ERR] %s:%d: " msg, __FUNCTION__, __LINE__, ##args);	\
  exit(-1); \
  } while (0)

#endif /* UTILITY_H__ */
