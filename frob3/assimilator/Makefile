all:
	bash all.bash
setup:
	bash 10-create.bash
	sleep 120
	bash 20-upgrade.bash
	bash 25-restart.bash
	sleep 60
	bash 20-upgrade.bash
	head /tmp/temp.droplet.name /tmp/temp.droplet.ip
	echo OKAY
delete:
	bash 90-delete.bash
clean:
	rm -f _* ,* *.log
