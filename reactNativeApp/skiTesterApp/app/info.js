import React from 'react';
import { Modal, View, Text, Button, StyleSheet } from 'react-native';

export default function InfoModal({ visible, onClose }) {
  return (
    <Modal
      visible={visible}
      animationType="slide"
      transparent={true}
    >
      <View style={styles.modalContainer}>
        <View style={styles.modalContent}>
          <Text style={styles.title}>Info</Text>
          <Text style={styles.body}>
            Tämä sovellus on suunniteltu suksien testaukseen. Voit määrittää 
            suksien lukumäärän, kierrosten määrän, lämpötilan, lumen laadun ja 
            pohjan kovuuden asetuksissa. Sovellus tallentaa mittaustulokset ja 
            näyttää ne tulossivulla. Voit myös jakaa tulokset PDF-muodossa.
            {"\n"}
            Sovellus on suunniteltu käytettäväksi wifi-lähettimen ja kahden 
            mittauskennon kanssa.
          </Text>
          <Button title="Sulje" onPress={onClose} />
        </View>
      </View>
    </Modal>
  );
}

const styles = StyleSheet.create({
  modalContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: 'rgba(0, 0, 0, 0.5)',
  },
  modalContent: {
    width: '80%',
    padding: 20,
    backgroundColor: 'white',
    borderRadius: 10,
    shadowColor: '#000',
    shadowOpacity: 0.25,
    shadowRadius: 4,
    elevation: 5,
  },
  title: {
    fontSize: 20,
    fontWeight: 'bold',
    marginBottom: 10,
  },
  body: {
    fontSize: 16,
    marginBottom: 20,
  },
});