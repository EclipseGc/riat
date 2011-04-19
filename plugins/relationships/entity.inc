<?php

$plugin = array(
  'name' => t('Entity'),
  'description' => t('Provides a plugin defining relationships to entities'),
  'base' => TRUE,
  'restrictions' => 'riat_entity_restrictions',
  'get child' => 'riat_entity_get_child',
  'get children' => 'riat_entity_get_children',
);

function riat_entity_get_child($plugin, $parent, $child) {
  $plugins = riat_entity_get_children($plugin, $parent);
  return $plugins[$parent . ':' . $child];
}

function riat_entity_get_children($plugin, $parent) {
  $entities = entity_get_info();
  $plugins = array();
  foreach ($entities as $entity_type => $entity) {
    $plugin['name'] = $entity['label'];
    $plugin['description'] = t('Provides a plugin defining relationships to a @entity entity.', array('@entity' => $entity_type));
    $plugin_id = $parent . ':' . $entity_type;
    drupal_alter('riat_entity_relationship', $plugin, $entity, $plugin_id);
    $plugins[$plugin_id] = $plugin;
  }
  drupal_alter('riat_entity_relationships', $plugins);
  return $plugins;
}

function riat_entity_restrictions($plugin, $plugin_type) {
  $entity = entity_get_info($plugin_type);
  $allowed_types = array();
  if (isset($entity['bundles'])) {
    foreach($entity['bundles'] as $type => $bundle) {
      $allowed_types[$type] = array('label' => $bundle['label'], 'parent restriction' => array());
    }
  }
  drupal_alter('riat_entity_restrictions', $allowed_types, $plugin_type);
  return $allowed_types;
}
