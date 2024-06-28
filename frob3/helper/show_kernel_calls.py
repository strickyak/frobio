import re, sys

t=''  # front tabs

RETURN = re.compile('^RETURN ')
CALL = re.compile(' OS9KERNEL([01]): ')

for line in sys.stdin:
	line = line.rstrip()
	_r = RETURN.match(line)
	_c =  CALL.search(line)
	if _r:
		# return
		line = line.replace('RETURN OKAY: OS9KERNEL', '<---- t')
		line = line.replace('RETURN ', '<-- ')
		print(t + line)
		if t: t = t[4:]

	elif _c:
		# call
		line = line.replace('OS9KERNEL', 't')
		task = _c.group(1)
		if task == '1': t = ''
		print(t + line)
		t = ':   ' + t
	else:
		pass
