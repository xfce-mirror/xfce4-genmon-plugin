scripts = [
  'datetime',
  'disktemp',
  'dkspuse',
  'gmail',
  'migrate_to_xfconf.sh',
  'monBandwidth',
  'monBat',
  'monCPU',
  'monTime',
  'monUSB',
  'monWIFI',
  'sysstat',
]

scripts += configure_file(
  configuration: configuration_data({
    'OPEN_CMD': open_cmd,
  }),
  input: 'googcal.in',
  output: 'googcal',
  install: false,
)

foreach script : scripts
  install_data(
    script,
    install_dir: get_option('prefix') / get_option('datadir') / 'xfce4' / 'genmon' / 'scripts',
  )
endforeach
