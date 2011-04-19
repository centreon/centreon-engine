#!/usr/bin/php
<?php

error_reporting(E_ALL);

define('NL', "\n");

define('BIN_NAGIOS', '/home/merethis/nagios/nagios');
define('BIN_CENTENGINE', '/home/merethis/engine/centengine');

define('BIN_NAGIOSTATS', '/home/merethis/nagios/nagiostats');
define('BIN_CENTENGINESTATS', '/home/merethis/engine/centenginestats');

define('FILE_NAGIOS_CONFIGURATION', '/home/merethis/nagios/etc/nagios.cfg');
define('FILE_CENTENGINE_CONFIGURATION', '/home/merethis/nagios/etc/nagios.cfg');

define('FILE_NAGIOS_SERVICES', '/home/merethis/nagios/etc/objects/services.cfg');
define('FILE_CENTENGINE_SERVICES', '/home/merethis/nagios/etc/objects/services.cfg');

define('DIR_NAGIOS_REPORT', '/home/merethis/nagios/log');
define('DIR_CENTENGINE_REPORT', '/home/merethis/engine/log');

define('DIR_NAGIOS_VAR', '/home/merethis/nagios/var');
define('DIR_CENTENGINE_VAR', '/home/merethis/nagios/var');

function usage($appname)
{
  exit('usage: '.$appname.' time nb_check'.NL);
}

function my_kill($binary)
{
  if (($ret = my_system('ps aux | grep '.basename($binary).' | grep -v grep')) != '')
    if (preg_match('#^\d{1,}\s*(\d{1,})#', $ret, $matche) === 1)
	my_system('kill -TERM '.$matche[1]);
}

function my_system($cmd)
{
  ob_start();
  passthru($cmd);
  $ret = ob_get_contents();
  ob_end_clean();
  return ($ret);
}

function get_memory($binary)
{
  if (($ret = my_system('ps aux | grep '.basename($binary))) == '')
    return (FALSE);
    $ref = $ret;
    do
      {
	$ret = $ref;
	$ref = preg_replace('#\s{2,}#U', ' ', $ret);
      }
    while($ref != $ret);
    $ret = explode(' ', $ret);
    return ($ret[4]);
}

function get_loadavg()
{
  if (($file_loadavg = file_get_contents('/proc/loadavg')) === FALSE)
    return (FALSE);
  if (preg_match('#^[0-9.]{1,} ([0-9.]{1,}) ([0-9.]{1,}) [0-9]{1,}/[0-9]{1,} [0-9]{1,}$#', $file_loadavg, $matche) !== 1)
    return (FALSE);
  return (array('load5' => $matche[1], 'load15' => $matche[2]));
}

function get_latency($binary)
{
  $ret = my_system($binary);
  if (preg_match('#Active Service Latency:\s*[0-9.]* / [0-9.]* / ([0-9.]*) sec#U', $ret, $matche) !== 1)
    return (FALSE);
  return ($matche[1]);
}

function generate_configuration($nb_check)
{
  if ($nb_check < 1)
    return (FALSE);

  $file = '';
  for ($i = 0; $i < $nb_check; ++$i)
    {
      $sleep_time = rand(1, 10);
      $service = '
define service{
  host_name             localhost
  service_description   autogen_sleep_'.$sleep_time.'
  check_command         check-service-alive
  use                   generic-service
  _SLEEP                '.$sleep_time.'
}';
      $file .= $service.NL;
    }

  return (trim($file));
}

function run_check($file_services, $file_configuration, &$data_autoconf, $binary, $binary_stats, $file_report, $var_dir, $nb_check, $time)
{
    if (file_put_contents($file_services, $data_autoconf) === FALSE)
      exit('error: file_put_contents failed'.NL);

    my_system('rm -rf '.$var_dir);
    @mkdir($var_dir);
    @mkdir($var_dir.'/rw');
    @mkdir($var_dir.'/spool');

    system($binary.' '.$file_configuration.' > /dev/null 2>&1 &');
    sleep(1);
    if (my_system('ps aux | grep '.basename($binary).' | grep -v grep') == '')
      exit('error: '.basename($binary).' not running'.NL);

    for ($i = 0; $i < $time; $i += 5 * 60)
      {
	$memory = get_memory($binary);
	$loadavg = get_loadavg();
	$latency = get_latency($binary_stats);

	$line = $nb_check.' '.$memory.' '.$loadavg['load5'].' '.$loadavg['load15'].' '.$latency.' '.time().NL;
	if (file_put_contents($file_report, $line, FILE_APPEND) === FALSE)
	  exit('error: file_put_contents failed'.NL);
	sleep(5 * 60);
      }

    my_kill($binary);
    return ($latency);
}

if ($argc != 3 || is_numeric($argv[1]) === FALSE || is_numeric($argv[2]) === FALSE)
  usage($argv[0]);

my_system('pkill -TERM '.basename(BIN_NAGIOS));
my_system('pkill -TERM '.basename(BIN_CENTENGINE));

$time = intval($argv[1]);
$nb_check = intval($argv[2]);
$nagios_latency = 0;
$centengine_latency = 0;
$timeout = $time / 2;

while ($nagios_latency < $timeout || $centengine_latency < $timeout)
  {
    if (($config = generate_configuration($nb_check)) === FALSE)
      exit('error: generate_configuration failed'.NL);

    $filename_report = date('m_d_y_H_i_s').'_'.$nb_check;

    if ($nagios_latency < $timeout)
       $nagios_latency = run_check(FILE_NAGIOS_SERVICES,
                                   FILE_NAGIOS_CONFIGURATION,
				   $config,
				   BIN_NAGIOS,
				   BIN_NAGIOSTATS,
				   DIR_NAGIOS_REPORT.'/'.$filename_report,
                                   DIR_NAGIOS_VAR,
				   $nb_check,
                                   $time);

    if ($centengine_latency < $timeout)
      $centengine_latency = run_check(FILE_CENTENGINE_SERVICES,
                                         FILE_CENTENGINE_CONFIGURATION,
					 $config,
					 BIN_CENTENGINE,
					 BIN_CENTENGINESTATS,
					 DIR_CENTENGINE_REPORT.'/'.$filename_report,
                                         DIR_CENTENGINE_VAR,
					 $nb_check,
                                         $time);
    $nb_check *= 2;
  }

?>
