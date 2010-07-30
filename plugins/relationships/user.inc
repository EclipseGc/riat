<?php

$plugin = array(
  'name' => t('User'),
  'description' => t('Provides a plugin defining relationships to users'),
  'base' => TRUE,
  'allowed types' => 'riat_user_allowed_types',
  'allowed parents' => 'riat_user_allowed_parents',
);

function riat_user_allowed_types() {
  $allowed_types = user_roles();
  unset($allowed_types[1]);
  drupal_alter('riat_allowed_types', $allowed_types, 'user');
  return $allowed_types;
}

function riat_user_allowed_parents() {
  $allowed_parents = array(
    'node',
    'user',
  );
  drupal_alter('riat_allowed_parent', $allowed_parents, 'user');
  return $allowed_parents;
}