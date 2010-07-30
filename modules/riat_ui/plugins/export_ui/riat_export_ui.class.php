<?php

class riat_export_ui extends ctools_export_ui {
  /**
   * Main entry point to manage a relationship.
   */
  function manage_page($js, $input, $item, $step = NULL) {
    drupal_set_title($this->get_page_title('manage', $item));

    // Check to see if there is a cached item to get if we're using the wizard.
    if (!empty($this->plugin['use wizard'])) {
      $cached = $this->edit_cache_get($item, 'edit');
      if (!empty($cached)) {
        $item = $cached;
      }
    }

    $form_state = array(
      'plugin' => $this->plugin,
      'object' => &$this,
      'ajax' => $js,
      'item' => $item,
      'op' => 'manage',
      'form type' => 'manage',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      'step' => $step,
      // Store these in case additional args are needed.
      'function args' => func_get_args(),
    );

    $output = $this->manage_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      $export_key = $this->plugin['export']['key'];
      drupal_goto(str_replace('%ctools_export_ui', $form_state['item']->{$export_key}, $this->plugin['redirect']['manage']));
    }

    return $output;
  }
  
  /**
   * Execute the form.
   *
   * Add and Edit both funnel into this, but they have a few different
   */
  function manage_execute_form(&$form_state) {
    if (!empty($this->plugin['use wizard'])) {
      return $this->manage_execute_form_wizard($form_state);
    }
    else {
      return $this->manage_execute_form_standard($form_state);
    }
  }

  /**
   * Execute the standard form for managing relationships.
   */
  function manage_execute_form_standard(&$form_state) {
    ctools_include('form');
    $output = ctools_build_form('riat_ui_relationship_item_form', $form_state);
    if (!empty($form_state['executed'])) {
      $this->manage_save_form($form_state);
    }

    return $output;
  }
  
  function manage_form(&$form, &$form_state) {
    $form['test'] = array(
      '#type' => 'textfield',
      '#title' => t('Test'),
    );
  }
}

function riat_ui_relationship_item_form(&$form_state) {
  $form = array();
  $form_state['object']->manage_form($form, $form_state);
  return $form;
}