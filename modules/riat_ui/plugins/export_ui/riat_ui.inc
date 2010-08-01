<?php

$plugin = array(
  'export' => array(
    'key' => 'name',
  ),
  'schema' => 'riat_definition',
  'access' => 'administer riat relationships',
  'menu' => array(
    'menu prefix' => 'admin/build',
    'menu item' => 'relationships',
    'menu title' => 'Relationships',
    'menu description' => 'Provides a mechanism for building relationship templates for future use.',
    'items' => array(
      'manage' => array(
        'path' => 'list/%ctools_export_ui/manage',
        'title' => 'Manage',
        'page callback' => 'ctools_export_ui_switcher_page',
        'page arguments' => array('riat_ui', 'manage', 5),
        'load arguments' => array('riat_ui'),
        'access callback' => 'ctools_export_ui_task_access',
        'access arguments' => array('riat_ui', 'manage', 5),
        'type' => MENU_CALLBACK,
      ),
    ),
  ),
  'title singular' => t('relationship'),
  'title singular proper' => t('Relationship'),
  'title plural' => t('relationships'),
  'title plural proper' => t('Relationships'),
  'form' => array(
    'settings' => 'riat_ui_ctools_export_ui_form',
    'validate' => 'riat_ui_ctools_export_ui_form_validate',
    'submit' => 'riat_ui_ctools_export_ui_form_submit',
  ),
  'handler' => array(
     'class' => 'riat_export_ui',
     'parent' => 'ctools_export_ui',
   ),
  'allowed operations' => array(
    'manage' => array(
      'title' => t('Manage'),
    )
  ),
  'redirect' => array(
    'manage' => 'admin/build/relationships',
  ),
);

function riat_ui_ctools_export_ui_form(&$form, &$form_state) {
  ctools_include('plugins');
  $relationship_types = ctools_get_plugins('riat', 'relationships');
  $form['info']['name']['#title'] = t('Type');
  $form['info']['name']['#description'] = t('The machine-readable name of this relationship template. This text will be used for constructing the URL of the create content  page for this relationship template. This name must contain only lowercase letters, numbers, and underscores. Underscores will be converted into hyphens when constructing the URL of the create content page. This name must be unique and cannot conflict with any existing content type.');
  $form['title'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#weight' => -10,
    '#description' => t('The human-readable name of this relationship template. This text will be displayed as part of the list on the create content page. It is recommended that this name begin with a capital letter and contain only letters, numbers, and spaces. This name must be unique and cannot conflict with any existing content type.'),
    '#default_value' => $form_state['item']->title,
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#description' => t('A brief description of this relationship template. This text will be displayed as part of the list on the create content page.'),
    '#default_value' => $form_state['item']->description,
  );
  $options = array();
  foreach ($relationship_types as $key => $relationship) {
    if ($relationship['base']) {
      $options[$key] = $relationship['name'];
    }
  }
  $form['item']['object'] = array(
    '#type' => 'select',
    '#title' => t('Object'),
    '#description' => t('Base objects are relationship definitions that are allowed to start a relationship template.'),
    '#options' => $options,
    '#default_value' => $form_state['item']->object,
  );
  ctools_include('dependent');
  foreach ($options as $key => $option) {
    $form['item']['type'][$key] = array(
      '#type' => 'select',
      '#title' => t($option .' Type'),
      '#description' => t('The '. $option .' type to create'),
      '#options' => $relationship_types[$key]['allowed types'](),
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array('edit-object' => array($key)),
      '#default_value' => $form_state['item']->object_type,
    );
  }
  $form['relationships'] = array(
    '#type' => 'value',
    '#value' => $form_state['item']->relationships,
  );
}

function riat_ui_ctools_export_ui_form_validate(&$form, &$form_state) {
  $form_state['values']['object_type'] = $form_state['values'][$form_state['values']['object']];
}

function riat_ui_ctools_export_ui_form_submit(&$form, &$form_state) {
  if ($form_state['values']['op'] == 'Save') {
    $relationships = $form_state['values']['relationships'];
    if ($relationships){
      foreach ($relationships as $relationship) {
        $relationship['name'] = $form_state['values']['name'];
        db_query("INSERT INTO {riat_tree_definition} (chid, pchid, name, object, object_type, relationship, depth, weight) VALUES (%d, %d, '%s', '%s', '%s', '%s', %d, %d)", $relationship);
      }
    }
  }
}
