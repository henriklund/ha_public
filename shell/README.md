# reset_network.sh

In Home Assistant OS, place following two files in the /share folder (done either through SSH or using one of the available terminals in Home Assistant):
- [reset_network.sh](https://github.com/henriklund/ha_public/blob/main/shell/reset_network.sh)
- [nmcli_cmds](https://github.com/henriklund/ha_public/blob/main/shell/nmcli_cmds)

Remember to make reset_network.sh executable by issuing a _chmod +x reset_network.sh_ in a terminal.

If you ever clone a VM running the HA OS installation, or create another machine (virtual / physical) from backup, then simply start that new machine, wait for the console to become available and type _login_ and then hit <enter>.
Now you can reset your network setting using the command /mnt/data/supervisor/share/reset_network.sh. The script will remove any defined gateway, DNS and IP and also enable DHCP in the network settings. Then it wull save settings
and reboot the machine.

Please _do_ go the the console and issue the command login. Then in the normal shell issue the command nmcli and verify that the used network adapter is "Supervisor enp0s18". If not, amend the _reset_network.sh_ script accordingly.
