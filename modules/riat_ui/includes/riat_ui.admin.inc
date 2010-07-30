<?php

function riat_ui_admin_relationships() {
  $output = '';
  
  return $output;
}

function riat_ui_add_relationship($relationship = NULL) {
  
}

function riat_ui_delete_relationship($relationship) {
  db_query("DELETE FROM {riat_definition} WHERE name = '%s'", $relationship);
  db_query("DELETE FROM {riat_tree_definition} WHERE name = '%s'", $relationship);
}