
void ps1GetCurrentTime(int *month, int *day, int *year, int *hour, int *minute)
{
    if (month)  *month  = ps1SoftMonth;
    if (day)    *day    = ps1SoftDay;
    if (year)   *year   = ps1SoftYear;
    if (hour)   *hour   = ps1SoftHour;
    if (minute) *minute = ps1SoftMinute;
}

void ps1SetCurrentTime(int month, int day, int year, int hour, int minute)
{
    if (month >= 1 && month <= 12)  ps1SoftMonth  = month;
    if (day >= 1 && day <= 31)      ps1SoftDay    = day;
    if (year >= 1990 && year <= 2099) ps1SoftYear = year;
    if (hour >= 0 && hour <= 23)    ps1SoftHour   = hour;
    if (minute >= 0 && minute <= 59) ps1SoftMinute = minute;
}

/* --- Info accessors --- */

const char *ps1GetCurrentAdsName(void)
{
    return ps1AdsCurrentName;
