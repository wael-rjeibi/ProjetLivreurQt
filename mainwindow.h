#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QString>

// (Optionnel) Qt Charts si tu les utilises quelque part
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // ===================== LIVREURS =====================
    void on_btnLivreurAjouter_2_clicked();   // Ajouter / Enregistrer
    void on_btnLivreurModifier_clicked();    // Prépare l’édition et ouvre le formulaire
    void on_btnLivreurSupprimer_clicked();
    void on_tableLivreurs_clicked(const QModelIndex &index);
    void on_btnTrierLivreurs_clicked();
    void rechercherLivreursParNom(const QString &texte);
    void on_btnExporterPDF_clicked();

    // ===================== VEHICULES =====================
    void on_btnVehiculeAjouter_clicked();
    void on_btnVehiculeModifier_clicked();
    void on_btnVehiculeSupprimer_clicked();
    void on_tableVehicules_clicked(const QModelIndex &index);

    // ===================== AUTRES =====================
    void on_btnAfficherVehicules_clicked();
    void on_btnAfficherStats_clicked();

private:
    Ui::MainWindow *ui;

    // ===================== MODELES =====================
    QStandardItemModel *vehiculeModel {nullptr};
    QStandardItemModel *livreurModel {nullptr};

    // ===================== SELECTIONS =====================
    QModelIndex selectedVehiculeIndex;
    QModelIndex selectedLivreurIndex;

    // Mode édition pour Livreur
    bool isEditingLivreur {false};

    // ===================== FONCTIONS INTERNES =====================
    void afficherDrivers();

    // Livreur
    void chargerLivreursDepuisBD();

    // Véhicule
    void chargerVehiculesDepuisBD();
    void chargerVehiculesPourLivreur(const QString& livreurId);

    // Statistiques
    void afficherStats();

    // ⭐ Reselectionner une ligne par ID après UPDATE
    void selectLivreurRowById(int id);
};

#endif // MAINWINDOW_H
