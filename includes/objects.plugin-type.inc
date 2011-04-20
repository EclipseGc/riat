<?php

/**
 * @file
 * Contains plugin type registration information for the object plugin tool.
 */

function riat_objects_plugin_type(&$items) {
  $items['objects'] = array(
    'child plugins' => TRUE,
  );
}
