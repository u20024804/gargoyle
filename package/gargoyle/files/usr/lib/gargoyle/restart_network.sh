# This program is copyright � 2008 Eric Bishop and is distributed under the terms of the GNU GPL 
# version 2.0 with a special clarification/exception that permits adapting the program to 
# configure proprietary "back end" software provided that all modifications to the web interface
# itself remain covered by the GPL. 
# See http://gargoyle-router.com/faq.html#qfoss for more information

# This script will effictively restart things if the wifi chip is brcm
# but things get completely FUBAR if the wifi is atheros
# If the wifi is Atheros, the router HAS to be restarted

# for some reason network interfaces are not all down
# after running "/etc/init.d/network stop",
# so we have to explicitly take all of them down 
# after stopping the network
# However, if vlan is active it can be problematic to take
# the switch interface down, so just take down the sub-interfaces

switch_if=$(uci show network | grep switch | sed "s/network\.//g" | sed "s/\=.*//g")
vlan_active=$(cat /proc/net/dev | grep "$switch_if\.")
ifs=$(cat /proc/net/dev 2>/dev/null | awk 'BEGIN {FS = ":"}; $0 ~ /:/ { print $1 }')

webmon_enabled=$(ls /etc/rc.d/*webmon_gargoyle 2>/dev/null)
dnsmasq_enabled=$(ls /etc/rc.d/*dnsmasq 2>/dev/null)
bwmon_enabled=$(ls /etc/rc.d/*bwmon_gargoyle 2>/dev/null)
qos_enabled=$(ls /etc/rc.d/*qos_gargoyle 2>/dev/null)


#stop firewall,dnsmasq,qos,bwmon,webmon
if [ -n "$webmon_enabled" ] ; then
	/etc/init.d/webmon_gargoyle stop >/dev/null 2>&1
fi
if [ -n "$bwmon_enabled" ] ; then
	/etc/init.d/bwmon_gargoyle stop >/dev/null 2>&1
fi
if [ -n "$qos_enabled" ] ; then
	/etc/init.d/qos_gargoyle stop >/dev/null 2>&1
fi
dump_quotas >/dev/null 2>&1
/etc/init.d/firewall stop >/dev/null 2>&1 
/etc/init.d/dnsmasq stop >/dev/null 2>&1 


#make sure any atheros wireless interfaces are destroyed correctly
#this didn't work for a while, but now that I've finally
#gotten around to fixing it, the default seems to work too
#include this here just to BE SURE
aths=$(ifconfig | sed 's/://g' | grep "^ath" | awk ' { print $1 }' )
if [ -n "$aths" ] ; then
	killall wpa_supplicant 2>/dev/null
	killall hostapd 2>/dev/null

	for ath in $aths ; do
		#reset txpower to max
		iwconfig $ath txpower 18dBm
		
		#remove from bridge if necessary
		brg=$(brctl show | grep "$ath" | awk '{ print $1 }')
		#echo "brg=$brg"
		if [ -n "$brg" ] ; then
			#echo brctl delif "$brg" "$ath" 
			brctl delif "$brg" "$ath" 
		fi
		ifconfig $ath down
		wlanconfig $ath destroy
	done
fi


/etc/init.d/network stop >/dev/null 2>&1 

for i in $ifs ; do
	is_switch=$(echo "$i" | egrep "$switch_if[^\.]*$")
	if [ -z "$is_switch" ] || [ -z "$vlan_active" ] ; then
		ifconfig $i down 2>/dev/null
	fi
done

/etc/init.d/network start >/dev/null 2>&1 


#restart everything
/etc/init.d/firewall start >/dev/null 2>&1
if [ -n "$qos_enabled" ] ; then
	/etc/init.d/qos_gargoyle start
fi
if [ -n "$bwmon_enabled" ] ; then
	/etc/init.d/bwmon_gargoyle start
fi
if [ -n "$dnsmasq_enabled" ] ; then
	/etc/init.d/dnsmasq start >/dev/null 2>&1 
fi
if [ -n "$webmon_enabled" ] ; then
	/etc/init.d/webmon_gargoyle start >/dev/null 2>&1
fi

lan_gateway=$(uci -P /var/state get network.lan.gateway)
wan_gateway=$(uci -P /var/state get network.wan.gateway)
if [ -n "$lan_gateway" ] ; then
	arping -c 2 -I br-lan $lan_gateway
	ping -c 2 $lan_gateway
fi
if [ -n "$wan_gateway" ] ; then
	arping -c 2 -I br-wan $wan_gateway
	ping -c 2 $wan_gateway
fi