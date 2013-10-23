<?php

function adef_camps_configure() {
  return drupal_get_form('adef_camps_configure_form');
}

function adef_camps_configure_form($form, &$form_state) {
  $form['start_date'] = array(
      '#type' => 'date',
      '#title' => t('Camps start date'),
      '#default_value' => variable_get('adef_camps_start_date', array(
          'year' => getdate()['year'],
          'month' => getdate()['mon'],
          'day' => getdate()['mday'],
      )),
  );
  $form['switch_hour'] = array(
      '#type' => 'textfield',
      '#title' => t('Switch hour'),
      '#description' => t('Which hour should the system use to switch to the next two day schedule. Use 24 hour format. Example: "22" for 10:00 PM.'),
      '#default_value' => strval(variable_get('adef_camps_switch_hour', 21)),
      '#size' => 2,
  );
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
  );
  return $form;
}

function adef_camps_configure_form_submit($form, &$form_state) {
  variable_set('adef_camps_start_date', $form_state['values']['start_date']);
  variable_set('adef_camps_switch_hour', intval($form_state['values']['switch_hour']));
  drupal_set_message(t('Configuration was saved.'));
}

function adef_camps_configure_form_validate($form, &$form_state) {
  $switch_hour = $form_state['values']['switch_hour'];
  $pass = TRUE;
  if (!is_numeric($switch_hour)) {
    $pass = FALSE;
  } else {
    $switch_hour = intval($switch_hour);
    if ($switch_hour > 23 || $switch_hour < 0)
      $pass = FALSE;
  }
  if (!$pass) {
    form_set_error('switch_hour', t('You must enter a number between 0 and 23.'));
  }
}