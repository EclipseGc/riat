<?php

/**
 * @file
 * Contains plugin type registration information for the riat_relationships tool.
 */

function riat_relationships_plugin_type(&$items) {
  $items['riat_relationships'] = array(
    'child plugins' => TRUE,
  );
}
