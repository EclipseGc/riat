<?php

$plugin = array(
  'name' => t('Node'),
  'description' => t('Provides a plugin defining relationships to nodes'),
  'base' => TRUE,
  'allowed types' => 'riat_node_allowed_types',
  'allowed parents' => 'riat_node_allowed_parents',
);

function riat_node_allowed_types() {
  $allowed_types = array();
  foreach(node_get_types() as $type) {
    $allowed_types[$type->type] = $type->name;
  }
  drupal_alter('riat_allowed_types', $allowed_types, 'node');
  return $allowed_types;
}

function riat_node_allowed_parents() {
  $allowed_parents = array(
    'node',
    'user',
  );
  drupal_alter('riat_allowed_parent', $allowed_parents, 'node');
  return $allowed_parents;
}