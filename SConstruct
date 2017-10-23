import os

env = Environment()

obj_file_path = 'build/obj'
exe_bin_path = 'build/run'
source_file_path = '.'
debug_symbols = True

libs = [
	'rt',
]

pkg_config_libs = [
]

env.Append(
	CCFLAGS = [
		'-g' if debug_symbols else None,
		'-Wall',
	],
	LINKFLAGS = [
	]
)

for lib in pkg_config_libs:
	env.ParseConfig('pkg-config --cflags --libs ' + lib);

def get_contents_of_dir(base):
	return [ os.path.abspath(os.path.join(base, i)) for i in os.listdir(base) if not i.startswith('.') ]

def get_subdirs(base):
	return [ i for i in get_contents_of_dir(base) if os.path.isdir(i) ]

def get_all_subdirs(base):
	return [base] + [i for j in get_subdirs(base) for i in get_all_subdirs(j)]

def has_extension(base, extensions):
	if type(extensions) != type([]):
		raise TypeError('has_extension must be sent a path and an array of extensions')
	for extension in extensions:
		if base.endswith(extension):
			return True
	return False

def get_all_files(base):
	return [ path for subdir in get_all_subdirs(base) for path in get_contents_of_dir(subdir) if os.path.isfile(path) ]

def get_all_files_with_extension(base, extensions):
	return [ path for path in get_all_files(base) if has_extension(path, extensions) ]

sources = get_all_files_with_extension(source_file_path, ['.cpp', '.c'])

objects = []

for source in sources:
	obj_path = os.path.join(obj_file_path, os.path.relpath(source.rsplit('.', 1)[0] + '.o'))
	obj = env.Object(target=obj_path, source=source)
	objects.append(obj)

program = env.Program(target=exe_bin_path, source=objects, LIBS=libs)
