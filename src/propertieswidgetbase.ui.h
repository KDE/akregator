

void FeedPropertiesWidgetBase::slotExpiryComboBoxActivated( int index)
{
    if (index == 0)
 expirySpinBox->setEnabled(false);
    else 
 expirySpinBox->setEnabled(true);
}
