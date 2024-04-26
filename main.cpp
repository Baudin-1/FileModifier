#include <QApplication>
#include <QWidget>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QTextCodec>
#include <QTimer>


void File_Modifier(const QString& mask, const bool &deleteFiles, const QString &outputPath, const bool &overwriteFiles, const qint64 &value); // Функция, модифицирующая файлы.

class FileModifier : public QWidget
{
	Q_OBJECT

public:
	FileModifier(QWidget *parent = nullptr) : QWidget(parent)
	{

		// Создаем элементы формы
		QLabel *maskLabel = new QLabel(QString::fromLocal8Bit("Маска входных файлов:"));
		QLineEdit *maskEdit = new QLineEdit(".txt");

		QLabel *deleteLabel = new QLabel(QString::fromLocal8Bit("Удалять входные файлы:"));
		QCheckBox *deleteCheck = new QCheckBox();

		QLabel *pathLabel = new QLabel(QString::fromLocal8Bit("Путь для сохранения файлов:"));
		QLineEdit *pathEdit = new QLineEdit();
		QPushButton *pathButton = new QPushButton(QString::fromLocal8Bit("Выбрать"));

		QLabel *actionLabel = new QLabel(QString::fromLocal8Bit("Действие при повторении имени файла:"));
		QCheckBox *actionCheck = new QCheckBox();

		QLabel *timerLabel = new QLabel(QString::fromLocal8Bit("Работа по таймеру:"));
		QCheckBox *timerCheck = new QCheckBox();

		QLabel *intervalLabel = new QLabel(QString::fromLocal8Bit("Периодичность опроса наличия входного файла:"));
		QLineEdit *intervalEdit = new QLineEdit();

		QLabel *valueLabel = new QLabel(QString::fromLocal8Bit("Значение 8 байт для бинарной операции:"));
		QLineEdit *valueEdit = new QLineEdit();

		QPushButton *startButton = new QPushButton(QString::fromLocal8Bit("Старт"));

		// Располагаем элементы на форме
		QVBoxLayout *layout = new QVBoxLayout;
		layout->addWidget(maskLabel);
		layout->addWidget(maskEdit);
		layout->addWidget(deleteLabel);
		layout->addWidget(deleteCheck);
		layout->addWidget(pathLabel);
		layout->addWidget(pathEdit);
		layout->addWidget(pathButton);
		layout->addWidget(actionLabel);
		layout->addWidget(actionCheck);
		layout->addWidget(timerLabel);
		layout->addWidget(timerCheck);
		layout->addWidget(intervalLabel);
		layout->addWidget(intervalEdit);
		layout->addWidget(valueLabel);
		layout->addWidget(valueEdit);
		layout->addWidget(startButton);
		setLayout(layout);

		// Подключаем сигналы к слотам
		connect(pathButton, &QPushButton::clicked, [=]() {
			QString path = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Выберите папку для сохранения файлов"), QDir::homePath());
			if (!path.isEmpty()) {
				pathEdit->setText(path);
			}
		});

		connect(startButton, &QPushButton::clicked, [=]() {
    // Получаем информацию с формы
			QString mask = maskEdit->text(); 
			bool deleteFiles = deleteCheck->isChecked(); 
			QString outputPath = pathEdit->text(); 
			bool overwriteFiles = actionCheck->isChecked();
			bool useTimer = timerCheck->isChecked();
			int interval = intervalEdit->text().toInt();
			qint64 value = valueEdit->text().toLongLong();

			if (useTimer) {
				// Создаем таймер
				QTimer *timer = new QTimer(this);
				connect(timer, &QTimer::timeout, [=]() { 
					File_Modifier(mask, deleteFiles, outputPath, overwriteFiles, value);
				});
			}
			else {
				File_Modifier(mask, deleteFiles, outputPath, overwriteFiles, value);
			}
		});
		
	}
};


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	FileModifier fileModifier;
	fileModifier.show();

	return app.exec();
}

#include "main.moc"



void File_Modifier(const QString& mask, const bool &deleteFiles, const QString &outputPath, const bool &overwriteFiles, const qint64 &value){
	QDir inputDir(QDir::currentPath());
	QStringList filters;
	filters << mask;
	inputDir.setNameFilters(filters);
	QStringList inputFiles = inputDir.entryList(); // Получаем список файлов 

	foreach(const QString &fileName, inputFiles) {
		QString inputFilePath = inputDir.absoluteFilePath(fileName);

		QFile inputFile(inputFilePath);

		// Открываем входной файл 
		if (inputFile.open(QIODevice::ReadOnly)) {
			QByteArray data = inputFile.readAll();

			// Модифицируем данные
			for (int i = 0; i < 8; i++) {
				data[i] = data[i] ^ static_cast<char>((value >> (i * 8)) & 0xFF);
			}

			QString outputFilePath = outputPath + QDir::separator() + fileName;

			QFile outputFile(outputFilePath);

			// Проверяем необходимость перезаписывать файл
			if (overwriteFiles || !QFile::exists(outputFilePath)) {
				if (outputFile.open(QIODevice::WriteOnly)) {
					// Записываем данные
					outputFile.write(data);
					outputFile.close();

					// Удаляем входной файл при необходимости
					if (deleteFiles) {
						inputFile.remove();
					}
				}
				else {
					qDebug() << "Ошибка открытия файла для записи: " << outputFilePath;
				}
			}
			else {
				qDebug() << "Файл уже существует и флаг перезаписи не установлен: " << outputFilePath;
			}
		}
		else {
			qDebug() << "Ошибка открытия файла для чтения: " << inputFilePath;
		}
	}
}
