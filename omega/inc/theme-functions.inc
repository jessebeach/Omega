<?php
// $Id$

/**
 * @file
 * This file contains functions that must be accessed BOTH in template.php
 * AND in theme-settings.php
 * 
 * putting the functions in template.php alone is not enough when using an 
 * alternate administrative interface to call specific php functions related
 * to the Omega theme.
 */

/**
 * NINESIXTY - Contextually adds 960 Grid System classes.
 *
 * The first parameter passed is the *default class*. All other parameters must
 * be set in pairs like so: "$variable, 3". The variable can be anything
 * available within a template file and the integer is the width set for the
 * adjacent box containing that variable.
 *
 *  class="<?php print ns('grid-16', $var_a, 6); ?>"
 *
 * If $var_a contains data, the next parameter (integer) will be subtracted from
 * the default class. See the README.txt file.
 */
function ns() {
  $args = func_get_args();
  $default = array_shift($args);
  // Get the type of class, i.e., 'grid', 'pull', 'push', etc.
  // Also get the default unit for the type to be procesed and returned.
  list($type, $return_unit) = explode('-', $default);

  // Process the conditions.
  $flip_states = array('var' => 'int', 'int' => 'var');
  $state = 'var';
  foreach ($args as $arg) {
    if ($state == 'var') {
      $var_state = !empty($arg);
    }
    elseif ($var_state) {
      $return_unit = $return_unit - $arg;
    }
    $state = $flip_states[$state];
  }

  $output = '';
  // Anything below a value of 1 is not needed.
  if ($return_unit > 0) {
    $output = $type . '-' . $return_unit;
  }
  return $output;
}

/**
 * Stoopid function to just sort, then re-key an array
 */
function omega_order_reorder($o) {
	ksort($o);
	$n = array();
	foreach ($o AS $v) {
		$n[] = $v;
	}
	return $n;
}

/**
 * The static_region_builder() function takes a set of region configurations
 * and container width as arguments, and returns an array of classes that should 
 * be applied to the regions in that zone.
 * 
 * The function is called from process_zone() at the time of zone rendering 
 * in order to take all associated regions in a zone and pass to this function.
 * 
 * Variables are then assigned, and rendered in process_region() to the
 * region.tpl.php
 *
 * For a more advanced set of regions, dynamic_region_builder() will be used.
 */
function static_region_builder($regions, $container_width) {
  // let's cycle the region data, and determine what we have
  $region_classes = array();
  foreach ($regions AS $region => $info) {
    // if we do have content for this region, let's create it.
    $region_classes[$region] = array();
    if ($info['data']) {
      $region_classes[$region][] = ns('grid-' . $info['width']);
    }
    if (isset($info['spacing'])) {
      foreach ($info['spacing'] AS $attribute => $value) {
        if ($value) {
          $region_classes[$region][] = ns($attribute . '-' . $value);
        } 
      }
    }
  }
  return $region_classes;
}

/**
 * The _omega_dynamic_widths functions is used in dynamic_region_builder()
 * 
 * The functiont takes the max width for the region ($width), and then will 
 * depending on the presence of the related zones, subtract their width from
 * the overall width of the container.
 * 
 * This function only gets called on the primary region of a zone.
 * This can be seen in example in the "sidebar content sidebar" type zone, the 
 * content region would be the primary, and the sidebars would not.
 * 
 * @see dynamic_region_builder()
 */
function _omega_dynamic_widths($info, $regions, $container) {
  $width = $container;
  $conditions = $info['related'];
	foreach ($conditions AS $variable => $region) {
    if ((isset($regions[$variable]['data'])) && strlen($regions[$variable]['data']) > 0) {
      $width = $width - $region['width'];
      if ($regions[$variable]['spacing']['prefix'] > 0) {
        $width = $width - $regions[$variable]['spacing']['prefix'];
      }
      if ($regions[$variable]['spacing']['suffix'] > 0) {
        $width = $width - $regions[$variable]['spacing']['prefix'];
      }
    }
  }
  return $width;
}

/**
 * The dynamic_region_builder function will be used to pass important zones
 * like the content regions where the regions sent to the function MUST appear
 * inline, and advanced calculations need to be done in order to display the as such
 *
 * Stacked regions are not possible using this function, and should be passed through
 * static_region_builder() instead.
 */
function dynamic_region_builder($region_data, $container_width, $region_order) {
	$region_classes = array();
  // let's cycle the region data, and determine what we have
  foreach ($region_data AS $region => $info) {
  	$region_classes[$region] = array();
    // if we do have content for this region, let's create it.
    if (isset($info['data'])) {
      if (isset($info['primary'])) {
        $width = _omega_dynamic_widths($info, $region_data, $container_width);;
        $region_classes[$region][] = ns('grid-' . $width);
      }
      else {
        $width = $info['width'];
        $region_classes[$region][] = ns('grid-' . $width);
      }
      
	    if (isset($info['spacing'])) {
	      foreach ($info['spacing'] AS $attribute => $value) {
	        if ($value) {
	          $region_classes[$region][] = ns($attribute . '-' . $value);
	          $width = $width + $value;
	        }
	      }
	    }

      // assign push/pull classes
      $region_movement = _omega_dynamic_zones($region, $region_data, $region_order, $container_width);
      
      $pull = ns('pull-' . $region_movement['pull']);
      $push = ns('push-' . $region_movement['push']);
      
      
      if($pull) {
        $region_classes[$region][] = $pull; 
      }
      if($push) {
        $region_classes[$region][] = $push; 
      }
      
    }
    
  }
  return $region_classes;
}

function _omega_dynamic_zones($region, $regions, $order, $width) {
  $value = array(
    'pull' => 0,
    'push' => 0,
  );
  $actual_order = array_keys($regions);
  $actual_position = array_search($region, $actual_order);
  $desired_position = array_search($region, $order);
  	foreach($actual_order as $delta => $region_id) {
  		// don't react if the region is the one we are testing
  		if ($region != $region_id) {  
  			// get the actual position of the region we are testing against
  		  $t_actual = $delta;
  		  // get the desired positon of the region we are testing against.
	      $t_desired = array_search($region_id, $order);
	      
	      if($regions[$region_id]['data'] && ($t_actual < $actual_position) && ($t_desired > $desired_position)) {
	        $value['pull'] = $value['pull'] + omega_actual_region_width($regions[$region_id]);
	      }
  		  if($regions[$region_id]['data'] && ($t_actual > $actual_position) && ($t_desired < $desired_position)) {
          $value['push'] = $value['push'] + omega_actual_region_width($regions[$region_id]);
        }
        
        // possible solution for that edge cases mentioned above (position in the 
        // array doesn't change, but the position itself needs to.... hrm...)
        if ($value['push'] > 0 && $value['pull'] > 0) {
        	//$value['push'] = 0;
        	//$value['pull'] = $regions[$region]['width'];
        	if ($value['push'] > $value['pull']) {
        		$value['push'] = $value['push'] - $value['pull'];
            $value['pull'] = 0;
        	}
        	elseif ($value['push'] < $value['pull']) {
        		$value['pull'] = $value['pull'] - $value['push'];
        		$value['push'] = 0;
        	}
        	elseif ($value['push'] == $value['pull']) {
        		$value['push'] = 0;
        		$value['pull'] = 0;
        	}
        }
	    }
  	}
  return $value;
}

/**
 * 
 * The omega_actual_region_width() function takes the region info as an arg, 
 * then returns the computed value of the width, prefix AND suffix in order to 
 * have the "actual" width of the zone to run other computations against.
 */
function omega_actual_region_width($r) {
	return $r['width'] + $r['spacing']['prefix'] + $r['spacing']['suffix'];
}