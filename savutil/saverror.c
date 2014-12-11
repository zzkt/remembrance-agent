#include <savant.h>

static void (*SE)(int, char *);

void SavantError(int errcode, char *errstring)
{
  (*SE)(errcode, errstring);
}

void SetSavantError(void (*newSEp)(int, char *))
{
  SE = newSEp;
}
