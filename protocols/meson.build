wayland_scanner = find_program('wayland-scanner')

wayland_scanner_code = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.c',
	arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.h',
	arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

protocols = [
  'wlr-gamma-control-unstable-v1.xml',
  'wlr-output-management-unstable-v1.xml',
]

protos_src = []
protos_headers = []

foreach xml : protocols
	protos_src += wayland_scanner_code.process(xml)
	protos_headers += wayland_scanner_client.process(xml)
endforeach

lib_protos = static_library(
	'protos',
	protos_src + protos_headers,
	dependencies: [wayland_client]
)

protos = declare_dependency(
	link_with: lib_protos,
	sources: protos_headers,
)
