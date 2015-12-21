BEGIN {FS=","}
{ print $1","$2","strftime("%I:%M:%S %p %D", systime())}
#%Y-%m-%d %H:%M:%S %Z")}
#{ print strftime()","$1","$2}

