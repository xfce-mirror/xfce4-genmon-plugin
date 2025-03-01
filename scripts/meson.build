scripts = [
  'datetime',
  'disktemp',
  'dkspuse',
  'gmail',
  'googcal',
  'migrate_to_xfconf.sh',
  'monBandwidth',
  'monBat',
  'monCPU',
  'monTime',
  'monUSB',
  'monWIFI',
  'sysstat',
]

foreach script : scripts
  install_data(
    script,
    install_dir: get_option('prefix') / get_option('datadir') / 'xfce4' / 'genmon' / 'scripts',
  )
endforeach
