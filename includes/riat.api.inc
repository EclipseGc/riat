<?php

function riat_load_relationship_types($type = NULL) {
  $types = module_invoke_all('riat_relationships');
  if (is_null($type)) {
    return $types;
  }
  else {
    return $types[$type];
  }
}

/**
 * Loads various relationship elements.
 *
 * @type
 *  'full'
 *    Load an entire relationship tree, this is the default functionality.
 *  'branch'
 *    Loads only the relationship components within this tree that directly lead to this branch.  Does not load subsequent relationship components.
 *    @a3 = child id of desired component.
 *  'depth'
 *    Loads all relationship components to a certain depth.
 *    @a3 = desired depth.
 *  'single'
 *    Load JUST an individual chid's definition, plus the main relationship definition itself.
 *    @a3 = child id of desired component.
 */
function riat_load_relationship_tree($relationship, $type = 'full', $a3 = NULL) {
  $definition = riat_load_definitions($relationship);
  switch ($type) {
    case 'full':
      $query = db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' ORDER BY depth", $relationship);
      while ($result = db_fetch_object($query)) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'recursive':
      $args = array(
        'name' => $relationship,
        'chid' => $result->chid,
      );
       $definition->raw = riat_load_definition_relationships($relationship);
       $results = array();
      riat_recursive_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND pchid = %d", $args, 'chid', $results);
      foreach ($results as $result) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->name .'_'. $result->chid] = $result;
      }
      break;
    case 'branch':
      $args = array(
        'name' => $relationship,
        'pchid' => $a3,
      );
      $results = array();
      riat_recursive_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d", $args, 'pchid', $results);
      foreach ($results as $result) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'depth':
      $query = db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND depth <= %d", $relationship, $a3);
      while ($result = db_fetch_object($query)) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'single':
      $result = db_fetch_object(db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d", $relationship, $a3));
      $definition->raw[$result->chid] = $result;
  }
  drupal_alter('riat_relationship_definition', $definition, $type, $relationship, $a3);
  return $definition;
}

/**
 * Generically loads up all direct relationships of an object by id.
 *
 * @object
 *  A string like 'node' or 'user'.
 * @id
 *  The unique id of the above object type.
 */
function riat_load_object_relationships($object, $id) {
  $results = array();
  $query = db_query("SELECT * FROM {riat_direct_relationship} WHERE object = '%s', id = %d", $object, $id);
  while ($result = db_fetch_object($query)) {
    $results[$result->name] = $result;
  }
  return $results;
}

/**
 * A handy generic function for doing simple recursive queries.
 * This function is designed to start some distance down a
 * hierarchical tree and work its way back on a single branch.
 * Top level arguments are expected to be 0, commonly following
 * the pchid element for most riat queries.
 *
 * @query_str
 *  The string intended to be run through db_query.  This query should only return a single result.
 * @args
 *  A keyed array of arguments to be passed to our query.  The same $key passed to us will be reset to new $result->$key of the query in $args[$key] = $result->$key format.
 * @key
 *  The key of the element to pass into subsequent queries as $args[$key].
 * @results
 *  An array of previous results passed by reference.
 */

function riat_recursive_query($query_str, $args, $key, &$results) {
  $query = db_query($query_str, $args);
  while ($result = db_fetch_object($query)) {
    $results[] = $result;
    if ($result->$key) {
      $args[$key] = $result->$key;
      riat_recursive_query($query_str, $args, $key, $results);
    }
  }
}

function riat_relationship_load($chid = NULL, $name) {
  if (!is_null($chid)) {
    if ($object = db_fetch_object(db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d", $name, $chid))) {
      return $object;
    }
    else {
      $relationship = riat_load_relationships($name .'_'. $chid);
      return $relationship ? $relationship : FALSE;
    }
  }
  else {
    $definitions = riat_load_relationships();
    foreach ($definitions as $relationship) {
      $relationships[$relationship->name .'_'. $relationship->chid] = $relationship;
    }
    $query = db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s'", $name);
    while ($result = db_fetch_object($query)) {
      $relationships[$result->name .'_'. $result->chid] = $result;
    }
    return $relationships;
  }
}

function riat_load_definition_relationships($relationship, $type = 'recursive') {
  switch ($type) {
    case 'recursive':
      $relationships = riat_relationship_load(NULL, $relationship);
      $relationships = riat_relationship_recursive_sort($relationships, $relationship);
      break;
    case 'high chid':
      $relationships = riat_relationship_load(NULL, $relationship);
      $chid = 0;
      foreach ($relationships as $relationship) {
        if ($relationship->chid > $chid) {
          $chid = $relationship->chid;
        }
      }
      return $chid;
  }
  return $relationships;
}

function riat_relationship_recursive_sort($items, $name, $pchid = 0) {
  $new_order = array();
  foreach ($items as $item) {
    if ($item->name == $name && $item->pchid == $pchid) {
      $new_order[$item->name .'_'. $item->chid] = $item;
      $new_order += riat_relationship_recursive_sort($items, $name, $item->chid);
    }
  }
  return $new_order;
}

function riat_get_relationships($filter = NULL, $key = NULL) {
  static $relationships;
  if (!$relationships) {
    $relationships = module_invoke_all('riat_relationships');
  }
  if (!is_null($key)) {
    return $relationships[$key];
  }
  $results = array();
  foreach ($relationships as $name => $relationship) {
    if ($filter) {
      $search_keys = array_keys($filter);
      foreach ($search_keys as $term) {
        if (isset($relationship[$term]) && $relationship[$term] == $filter[$term]) {
          $results[$name] = $relationship['title'];
        }
      }
    }
    else {
      $results[$name] = $relationship['title'];
    }
  }
  return $results;
}

function riat_relationship_list() {
  $list = array();
  $items = riat_load_relationships();

  foreach ($items as $item) {
    $string = $item->name .'_'. $item->chid;
    $list[$item->name .'_'. $item->chid] = check_plain($string);
  }
  return $list;
}

function riat_riat_definition_tree_load($name) {
  $explode = explode('_', $name);
  $chid = array_pop($explode);
  $name = implode('_', $explode);
  return riat_relationship_load($chid, $name);
}

function riat_relationship_to_hook_code($objects, $name) {
  $table = 'riat_tree_definition';
  $schema = drupal_get_schema($table);
  $export = $schema['export'];
  $output = '';
  $objects = ctools_export_load_object($table, 'all', $objects);
  if ($objects) {
    $output = "/**\n";
    $output .= " * Implementation of hook_{$export['default hook']}()\n";
    $output .= " */\n";
    $output .= "function " . $name . "_{$export['default hook']}() {\n";
    $output .= "  \${$export['identifier']}s = array();\n\n";
    foreach ($objects as $object) {
      $relationship = riat_load_relationships($object->name .'_'. $object->chid);
      $output .= ctools_export_crud_export($table, $relationship, '  ');
      $output .= "  \${$export['identifier']}s['" . check_plain($object->name .'_'. $object->chid) . "'] = \${$export['identifier']};\n\n";
    }
    $output .= "  return \${$export['identifier']}s;\n";
    $output .= "}\n";
  }

  return $output;
}

function riat_relationship_delete($object) {
  db_query("DELETE FROM {riat_definition} WHERE name = '%s'", $object->name);
  db_query("DELETE FROM {riat_tree_definition} WHERE name = '%s'", $object->name);
}

function riat_load_definitions($relationship = NULL) {
  static $cached = FALSE;
  static $definitions = array();
  if ($cached) {
    if (!is_null($relationship)) {
      return $definitions[$relationship];
    }
    else {
      return $definitions;
    }
  }
  $modules = module_list();
  foreach ($modules as $module) {
    $version = module_invoke($module, 'ctools_plugin_api', 'riat', 'default_riat_definition');
    if ($version) {
      module_load_include('inc', $module, "$module.default_riat_definition");
    }
  }
  $definitions = module_invoke_all('default_riat_definition');
  $objects = db_query("SELECT * FROM {riat_definition}");
  while ($result = db_fetch_object($objects)) {
    $definitions[$result->name] = $result;
  }
  foreach ($definitions as $key => $definition) {
    $relationships = riat_load_relationships($key);
    if ($relationships) {
      foreach ($relationships as $chid => $item) {
        $definitions[$key]->raw[$chid] = $item;
      }
    }
  }
  $cached = TRUE;
  if (!is_null($relationship)) {
    return $definitions[$relationship];
  }
  else {
    return $definitions;
  }
}

function riat_load_relationships($relationship = NULL) {
  static $cached = FALSE;
  static $relationships = array();
  if ($cached) {
    if (!is_null($relationship) && isset($relationships[$relationship])) {
      return $relationships[$relationship];
    }
    else {
      return $relationships;
    }
  }
  $modules = module_list();
  foreach ($modules as $module) {
    $version = module_invoke($module, 'ctools_plugin_api', 'riat', 'default_riat_tree');
    if ($version) {
      module_load_include('inc', $module, "$module.default_riat_tree");
    }
  }
  $relationships = module_invoke_all('default_riat_tree');
  $objects = db_query("SELECT * FROM {riat_tree_definition}");
  while ($result = db_fetch_object($objects)) {
    unset($result->rid);
    $relationships[$result->name .'_'. $result->chid] = $result;
  }
  $cached = TRUE;
  if (!is_null($relationship)) {
    return $relationships[$relationship];
  }
  else {
    return $relationships;
  }
}
