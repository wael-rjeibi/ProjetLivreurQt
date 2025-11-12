#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QtPrintSupport/QPrinter>
#include <QPainter>
#include <QFileDialog>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QDate>
#include <QMessageBox>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QDebug>
#include <QHeaderView>
#include <QVariant>

// ================== CONSTRUCTEUR ==================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    isEditingLivreur = false;

    // ID en lecture seule pour éviter des incohérences lors d'une édition
    ui->lineClientId_2->setReadOnly(true);
    // Libellé par défaut du bouton d’ajout
    ui->btnLivreurAjouter_2->setText("Ajouter");

    afficherDrivers();

    // ====== TABLE VÉHICULES (utilise tableCommandes_2) ======
    vehiculeModel = new QStandardItemModel(this);
    vehiculeModel->setColumnCount(4);
    vehiculeModel->setHeaderData(0, Qt::Horizontal, "ID");
    vehiculeModel->setHeaderData(1, Qt::Horizontal, "MARQUE");
    vehiculeModel->setHeaderData(2, Qt::Horizontal, "MATRICULATION");
    vehiculeModel->setHeaderData(3, Qt::Horizontal, "ANNEE_VEHICULE");
    // Tri numérique via Qt::UserRole
    vehiculeModel->setSortRole(Qt::UserRole);

    ui->tableCommandes_2->setModel(vehiculeModel);
    ui->tableCommandes_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableCommandes_2->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableCommandes_2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableCommandes_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableCommandes_2->setSortingEnabled(true);
    connect(ui->tableCommandes_2, &QTableView::clicked,
            this, &MainWindow::on_tableVehicules_clicked);

    // ====== TABLE LIVREURS (utilise tableClients_2) ======
    livreurModel = new QStandardItemModel(this);
    livreurModel->setColumnCount(4);
    livreurModel->setHeaderData(0, Qt::Horizontal, "ID");
    livreurModel->setHeaderData(1, Qt::Horizontal, "Nom");
    livreurModel->setHeaderData(2, Qt::Horizontal, "Prénom");
    livreurModel->setHeaderData(3, Qt::Horizontal, "Expérience");
    // Tri numérique via Qt::UserRole
    livreurModel->setSortRole(Qt::UserRole);

    ui->tableClients_2->setModel(livreurModel);
    ui->tableClients_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableClients_2->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableClients_2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableClients_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableClients_2->setSortingEnabled(true);

    connect(ui->tableClients_2, &QTableView::clicked,
            this, &MainWindow::on_tableLivreurs_clicked);

    // Boutons / Entrées
    connect(ui->btnTrierLivreurs, &QPushButton::clicked,
            this, &MainWindow::on_btnTrierLivreurs_clicked);
    connect(ui->rechercherLivreursParNom, &QLineEdit::textChanged,
            this, &MainWindow::rechercherLivreursParNom);
    connect(ui->btnExporterPDF_clicked, &QPushButton::clicked,
            this, &MainWindow::on_btnExporterPDF_clicked);

    connect(ui->btnAfficherStats_clicked, &QPushButton::clicked,
            this, &MainWindow::on_btnAfficherStats_clicked);
    connect(ui->btnAfficherVehicules, &QPushButton::clicked,
            this, &MainWindow::on_btnAfficherVehicules_clicked);

    // Ajout / Enregistrer livreur
    connect(ui->btnLivreurAjouter_2, &QPushButton::clicked,
            this, &MainWindow::on_btnLivreurAjouter_2_clicked);

    // Modifier livreur
    connect(ui->btnLivreurModifier, &QPushButton::clicked,
            this, &MainWindow::on_btnLivreurModifier_clicked);

    // spinClientAge_2 = expérience du livreur (on garde ton nom)
    ui->spinClientAge_2->setMinimum(0);

    if (QSqlDatabase::database().isOpen()) {
        chargerLivreursDepuisBD();   // ⇦ tri initial par EXPÉRIENCE (numérique)
        chargerVehiculesDepuisBD();
        ui->stackedWidget_2->setCurrentWidget(ui->pageClientForm_2);
    } else {
        QMessageBox::warning(this, "Oracle", "Connexion Oracle non ouverte.");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ================== PILOTES ==================
void MainWindow::afficherDrivers()
{
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << "Pilotes SQL disponibles :" << drivers;
    if (drivers.contains("QODBC")) {
        qDebug() << "✅ Pilote QODBC disponible.";
    } else {
        qDebug() << "❌ Pilote QODBC manquant.";
    }
}

// ================== HELPER reselection ==================
void MainWindow::selectLivreurRowById(int id)
{
    for (int r = 0; r < livreurModel->rowCount(); ++r) {
        if (livreurModel->item(r, 0)->data(Qt::UserRole).toInt() == id) {
            QModelIndex idx = livreurModel->index(r, 0);
            ui->tableClients_2->setCurrentIndex(idx);
            ui->tableClients_2->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            break;
        }
    }
}

// ================== CHARGEMENT LIVREURS ==================
void MainWindow::chargerLivreursDepuisBD()
{
    if (!QSqlDatabase::database().isOpen()) return;

    livreurModel->removeRows(0, livreurModel->rowCount());

    QSqlQuery q(QSqlDatabase::database());
    // Tri initial côté SQL (optionnel)
    if (!q.exec("SELECT ID, NOM, PRENOM, EXPERIENCE "
                "FROM LIVREUR ORDER BY EXPERIENCE ASC, ID ASC")) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }

    while (q.next()) {
        // ID
        const int idVal = q.value(0).toInt();
        auto *idItem = new QStandardItem(QString::number(idVal));
        idItem->setData(idVal, Qt::UserRole); // <- tri numérique

        // NOM
        auto *nomItem = new QStandardItem(q.value(1).toString());

        // PRENOM
        auto *prenomItem = new QStandardItem(q.value(2).toString());

        // EXPERIENCE
        const int expVal = q.value(3).toInt();
        auto *expItem = new QStandardItem(QString::number(expVal));
        expItem->setData(expVal, Qt::UserRole); // <- tri numérique

        QList<QStandardItem*> row;
        row << idItem << nomItem << prenomItem << expItem;
        livreurModel->appendRow(row);
    }
    q.finish();

    // Indicateur visuel + ordre
    ui->tableClients_2->sortByColumn(3, Qt::AscendingOrder);
    ui->tableClients_2->horizontalHeader()->setSortIndicator(3, Qt::AscendingOrder);

    // Reset de la sélection (évite des indices obsolètes)
    selectedLivreurIndex = QModelIndex();
}

// ================== CHARGEMENT VÉHICULES (toutes) ==================
void MainWindow::chargerVehiculesDepuisBD()
{
    if (!QSqlDatabase::database().isOpen()) return;

    vehiculeModel->removeRows(0, vehiculeModel->rowCount());

    QSqlQuery q(QSqlDatabase::database());
    if (!q.exec("SELECT ID, MARQUE, MATRICULATION, ANNEE_VEHICULE "
                "FROM VEHICULE ORDER BY ID")) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }
    while (q.next()) {
        // ID
        const int idVal = q.value(0).toInt();
        auto *idItem = new QStandardItem(QString::number(idVal));
        idItem->setData(idVal, Qt::UserRole); // <- tri numérique

        // MARQUE
        auto *marqueItem = new QStandardItem(q.value(1).toString());

        // MATRICULATION
        auto *matricItem = new QStandardItem(q.value(2).toString());

        // ANNEE_VEHICULE
        const int anneeVal = q.value(3).toInt();
        auto *anneeItem = new QStandardItem(QString::number(anneeVal));
        anneeItem->setData(anneeVal, Qt::UserRole); // <- tri numérique

        QList<QStandardItem*> row;
        row << idItem << marqueItem << matricItem << anneeItem;
        vehiculeModel->appendRow(row);
    }
    q.finish();
}

// ================== CHARGEMENT VÉHICULES (pour un livreur) ==================
void MainWindow::chargerVehiculesPourLivreur(const QString& livreurId)
{
    if (!QSqlDatabase::database().isOpen()) return;

    vehiculeModel->removeRows(0, vehiculeModel->rowCount());

    QSqlQuery q(QSqlDatabase::database());
    q.prepare("SELECT ID, MARQUE, MATRICULATION, ANNEE_VEHICULE "
              "FROM VEHICULE WHERE LIVREUR_ID = ? ORDER BY ID");
    q.addBindValue(livreurId);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }
    while (q.next()) {
        // ID
        const int idVal = q.value(0).toInt();
        auto *idItem = new QStandardItem(QString::number(idVal));
        idItem->setData(idVal, Qt::UserRole);

        // MARQUE
        auto *marqueItem = new QStandardItem(q.value(1).toString());

        // MATRICULATION
        auto *matricItem = new QStandardItem(q.value(2).toString());

        // ANNEE_VEHICULE
        const int anneeVal = q.value(3).toInt();
        auto *anneeItem = new QStandardItem(QString::number(anneeVal));
        anneeItem->setData(anneeVal, Qt::UserRole);

        QList<QStandardItem*> row;
        row << idItem << marqueItem << matricItem << anneeItem;
        vehiculeModel->appendRow(row);
    }
    q.finish();
}

// ================== AJOUT / MAJ LIVREUR ==================
void MainWindow::on_btnLivreurAjouter_2_clicked()
{
    const QString nom     = ui->lineClientNom_2->text().trimmed();
    const QString prenom  = ui->lineClientPrenom_2->text().trimmed();
    const int experience  = ui->spinClientAge_2->value();

   if (nom.isEmpty() || prenom.isEmpty()) {
       QMessageBox::warning(this, "livreur", "livreur ajouter");
       return;
    }

    QSqlQuery q(QSqlDatabase::database());
    bool okPrepare = false;
    int editedId = -1;

    if (isEditingLivreur) {
        // 🔒 UPDATE
        bool okId = false;
        editedId = ui->lineClientId_2->property("origIdInt").toInt(&okId);
        if (!okId) {
            editedId = ui->lineClientId_2->text().toInt(&okId);
        }
        if (!okId) {
            QMessageBox::warning(this, "Modification", "ID d'origine invalide.");
            return;
        }

        okPrepare = q.prepare("UPDATE LIVREUR SET NOM=?, PRENOM=?, EXPERIENCE=? WHERE ID=?");
        q.addBindValue(nom);
        q.addBindValue(prenom);
        q.addBindValue(experience);
        q.addBindValue(editedId);

        if (!okPrepare || !q.exec()) {
            QMessageBox::critical(this, "Erreur SQL (UPDATE)", q.lastError().text());
            return;
        }
        qDebug() << "UPDATE LIVREUR rowsAffected=" << q.numRowsAffected();

        // sortir du mode édition
        isEditingLivreur = false;
        ui->btnLivreurAjouter_2->setText("Ajouter");
        ui->lineClientId_2->setProperty("origIdInt", QVariant());
    } else {
        // ➕ INSERT
        okPrepare = q.prepare("INSERT INTO LIVREUR (NOM, PRENOM, EXPERIENCE) VALUES (?, ?, ?)");
        q.addBindValue(nom);
        q.addBindValue(prenom);
        q.addBindValue(experience);

        if (!okPrepare || !q.exec()) {
            QMessageBox::critical(this, "Erreur SQL (INSERT)", q.lastError().text());
            return;
        }
        // NB: Pour récupérer l'ID inséré sous Oracle il faut une SEQUENCE + RETURNING INTO.
    }

    // recharger et revenir au tableau
    chargerLivreursDepuisBD();
    ui->stackedWidget_2->setCurrentWidget(ui->pageClientTable_2);

    // reselectionner la ligne modifiée si on connaît l'ID
    if (editedId > -1) {
        selectLivreurRowById(editedId);
    }

    // reset du formulaire
    ui->lineClientId_2->clear();
    ui->lineClientNom_2->clear();
    ui->lineClientPrenom_2->clear();
    ui->spinClientAge_2->setValue(0);
    selectedLivreurIndex = QModelIndex();
}

// ================== MODIFIER LIVREUR (ouvre le formulaire d'ajout) ==================
void MainWindow::on_btnLivreurModifier_clicked()
{
    // 1) Ligne sélectionnée
    QModelIndex idx = selectedLivreurIndex.isValid()
                      ? selectedLivreurIndex
                      : ui->tableClients_2->currentIndex();

    if (!idx.isValid()) {
        QMessageBox::information(this, "Modifier",
                                 "Sélectionnez d'abord un livreur dans la table.");
        return;
    }

    const int row = idx.row();
    const int idInt     = livreurModel->item(row, 0)->data(Qt::UserRole).toInt();
    const QString nom   = livreurModel->item(row, 1)->text();
    const QString prenom= livreurModel->item(row, 2)->text();
    const int exp       = livreurModel->item(row, 3)->data(Qt::UserRole).toInt();

    // 2) Pré-remplir le formulaire d'ajout
    ui->lineClientId_2->setText(QString::number(idInt));
    ui->lineClientId_2->setProperty("origIdInt", idInt); // mémoriser ID d'origine (int)
    ui->lineClientNom_2->setText(nom);
    ui->lineClientPrenom_2->setText(prenom);
    ui->spinClientAge_2->setValue(exp);

    // 3) Passer en mode édition et aller sur la page du formulaire d'ajout
    isEditingLivreur = true;
    ui->btnLivreurAjouter_2->setText("Enregistrer"); // feedback visuel
    ui->stackedWidget_2->setCurrentWidget(ui->pageClientForm_2);
}

// ================== SUPPRIMER LIVREUR ==================
void MainWindow::on_btnLivreurSupprimer_clicked()
{
    QModelIndexList selection =
        ui->tableClients_2->selectionModel()
        ? ui->tableClients_2->selectionModel()->selectedRows()
        : QModelIndexList();

    if (selection.isEmpty()) {
        QMessageBox::information(this, "Suppression", "Veuillez sélectionner un livreur à supprimer.");
        return;
    }

    const int idInt = livreurModel->item(selection.first().row(), 0)->data(Qt::UserRole).toInt();

    if (QMessageBox::question(this, "Confirmation",
                              "Supprimer ce livreur ? (les véhicules liés seront supprimés)",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query(QSqlDatabase::database());
    query.prepare("DELETE FROM LIVREUR WHERE ID = ?");
    query.addBindValue(idInt); // ⭐ numérique

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur SQL", query.lastError().text());
        return;
    }

    chargerLivreursDepuisBD();
    chargerVehiculesDepuisBD();
}

// ================== AUTRES FONCTIONS ==================
void MainWindow::on_tableLivreurs_clicked(const QModelIndex &index)
{
    selectedLivreurIndex = index;
    const QString clientId = livreurModel->item(index.row(), 0)->text();
    chargerVehiculesPourLivreur(clientId);
}

void MainWindow::on_btnTrierLivreurs_clicked()
{
    // 🔀 Tri sur la colonne "Expérience" (index 3)
    static bool asc = false; // alterne à chaque clic
    asc = !asc;
    const Qt::SortOrder order = asc ? Qt::AscendingOrder : Qt::DescendingOrder;

    // Tri côté modèle (utilise UserRole) + visuel
    livreurModel->sort(3, order);
    ui->tableClients_2->sortByColumn(3, order);

    qDebug() << "[Tri livreurs] par EXPERIENCE, ordre =" << (asc ? "ASC" : "DESC");
}

void MainWindow::rechercherLivreursParNom(const QString &texte)
{
    for (int row = 0; row < livreurModel->rowCount(); ++row) {
        QString nom = livreurModel->item(row, 1)->text();
        ui->tableClients_2->setRowHidden(row, !nom.contains(texte, Qt::CaseInsensitive));
    }
}

// ================== EXPORT PDF ==================
void MainWindow::on_btnExporterPDF_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "*.pdf");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) fileName += ".pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier PDF.");
        return;
    }

    int y = 50;
    int rowHeight = 30;
    int colWidth = 120;

    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(0, y, "Liste des Livreurs");
    y += 40;

    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(0, y, "ID");
    painter.drawText(colWidth, y, "Nom");
    painter.drawText(colWidth * 2, y, "Prénom");
    painter.drawText(colWidth * 3, y, "Expérience");
    y += rowHeight;

    painter.setFont(QFont("Arial", 10));
    for (int row = 0; row < livreurModel->rowCount(); ++row) {
        painter.drawText(0, y, livreurModel->item(row, 0)->text());
        painter.drawText(colWidth, y, livreurModel->item(row, 1)->text());
        painter.drawText(colWidth * 2, y, livreurModel->item(row, 2)->text());
        painter.drawText(colWidth * 3, y, livreurModel->item(row, 3)->text());
        y += rowHeight;
    }

    painter.end();
    QMessageBox::information(this, "Succès", "PDF exporté avec succès !");
}

// ================== VÉHICULES : AJOUT ==================
void MainWindow::on_btnVehiculeAjouter_clicked()
{
    if (!selectedLivreurIndex.isValid()) {
        QMessageBox::information(this, "Info", "Sélectionnez un livreur avant d’ajouter un véhicule.");
        return;
    }

    QString clientId      = livreurModel->item(selectedLivreurIndex.row(), 0)->text();
    QString marque        = ui->lineCommandeTitre_2->text().trimmed();       // MARQUE
    QString matriculation = ui->textCommandeDesc_2->toPlainText().trimmed(); // MATRICULATION
    int annee             = ui->dateCommande_2->date().year();               // ANNEE_VEHICULE

    QSqlQuery q(QSqlDatabase::database());
    q.prepare("INSERT INTO VEHICULE (MARQUE, MATRICULATION, ANNEE_VEHICULE, LIVREUR_ID) "
              "VALUES (?, ?, ?, ?)");
    q.addBindValue(marque);
    q.addBindValue(matriculation);
    q.addBindValue(annee);
    q.addBindValue(clientId);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }

    chargerVehiculesPourLivreur(clientId);

    // reset formulaire (on garde tes noms actuels)
    ui->lineCommandeTitre_2->clear();
    ui->textCommandeDesc_2->clear();
    ui->dateCommande_2->setDate(QDate(QDate::currentDate().year(), 1, 1));
}

// ================== VÉHICULES : MODIFIER ==================
void MainWindow::on_btnVehiculeModifier_clicked()
{
    if (!selectedVehiculeIndex.isValid()) return;

    QString id            = vehiculeModel->item(selectedVehiculeIndex.row(), 0)->text();
    QString marque        = ui->lineCommandeTitre_2->text().trimmed();
    QString matriculation = ui->textCommandeDesc_2->toPlainText().trimmed();
    int annee             = ui->dateCommande_2->date().year();

    QSqlQuery q(QSqlDatabase::database());
    q.prepare("UPDATE VEHICULE "
              "SET MARQUE = ?, MATRICULATION = ?, ANNEE_VEHICULE = ? "
              "WHERE ID = ?");
    q.addBindValue(marque);
    q.addBindValue(matriculation);
    q.addBindValue(annee);
    q.addBindValue(id);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }

    if (selectedLivreurIndex.isValid())
        chargerVehiculesPourLivreur(livreurModel->item(selectedLivreurIndex.row(), 0)->text());
    else
        chargerVehiculesDepuisBD();
}

// ================== VÉHICULES : SUPPRIMER ==================
void MainWindow::on_btnVehiculeSupprimer_clicked()
{
    if (!selectedVehiculeIndex.isValid()) return;

    QString id = vehiculeModel->item(selectedVehiculeIndex.row(), 0)->text();

    if (QMessageBox::question(this, "Suppression",
                              "Supprimer ce véhicule ?",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    QSqlQuery q(QSqlDatabase::database());
    q.prepare("DELETE FROM VEHICULE WHERE ID = ?");
    q.addBindValue(id);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }

    if (selectedLivreurIndex.isValid())
        chargerVehiculesPourLivreur(livreurModel->item(selectedLivreurIndex.row(), 0)->text());
    else
        chargerVehiculesDepuisBD();
}

// ================== VÉHICULES : CLIC TABLE ==================
void MainWindow::on_tableVehicules_clicked(const QModelIndex &index)
{
    selectedVehiculeIndex = index;
    int row = index.row();

    // Remplit le formulaire en réutilisant tes widgets actuels
    ui->lineCommandeId_2->setText(vehiculeModel->item(row, 0)->text());
    ui->lineCommandeTitre_2->setText(vehiculeModel->item(row, 1)->text());
    ui->textCommandeDesc_2->setPlainText(vehiculeModel->item(row, 2)->text());

    bool ok = false;
    int annee = vehiculeModel->item(row, 3)->data(Qt::UserRole).toInt(&ok);
    ui->dateCommande_2->setDate(ok ? QDate(annee, 1, 1)
                                   : QDate(QDate::currentDate().year(), 1, 1));
}

// ================== AFFICHER VÉHICULES D’UN LIVREUR ==================
void MainWindow::on_btnAfficherVehicules_clicked()
{
    if (!selectedLivreurIndex.isValid()) {
        QMessageBox::information(this, "Information", "Sélectionnez un client d'abord.");
        return;
    }
    QString clientId = livreurModel->item(selectedLivreurIndex.row(), 0)->text();
    chargerVehiculesPourLivreur(clientId);
}

// ================== STATISTIQUES ==================
void MainWindow::afficherStats()
{
    auto getInt = [](const QString &queryStr) -> int {
        QSqlQuery q(QSqlDatabase::database());
        if (!q.exec(queryStr) || !q.next()) return 0;
        return q.value(0).toInt();
    };

    auto getDouble = [](const QString &queryStr) -> double {
        QSqlQuery q(QSqlDatabase::database());
        if (!q.exec(queryStr) || !q.next()) return 0.0;
        return q.value(0).toDouble();
    };

    int nbLivreurs   = getInt("SELECT COUNT(*) FROM LIVREUR");
    int nbVehicules  = getInt("SELECT COUNT(*) FROM VEHICULE");
    double expMoy    = getDouble("SELECT AVG(EXPERIENCE) FROM LIVREUR");

    QString msg;
    msg += "📊 Statistiques Générales\n\n";
    msg += QString("• Nombre de livreurs : %1\n").arg(nbLivreurs);
    msg += QString("• Nombre de véhicules : %1\n").arg(nbVehicules);
    msg += QString("• Expérience moyenne : %1 ans\n")
               .arg(QString::number(expMoy, 'f', 1));

    QMessageBox::information(this, "Statistiques", msg);
}

void MainWindow::on_btnAfficherStats_clicked()
{
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::warning(this, "Statistiques", "Connexion Oracle non ouverte.");
        return;
    }
    afficherStats();
}
