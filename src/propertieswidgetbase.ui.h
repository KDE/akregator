

void FeedPropertiesWidgetBase::slotUpdateComboBoxActivated( int index )
{
    if ( index == 3 ) // "never"
 updateSpinBox->setEnabled(false);
    else 
 updateSpinBox->setEnabled(true);
}


void FeedPropertiesWidgetBase::slotUpdateCheckBoxToggled( bool enabled )
{
    if (enabled && updateComboBox->currentItem() != 3 ) // "never"
 updateSpinBox->setEnabled(true);
    else 
 updateSpinBox->setEnabled(false);
}
