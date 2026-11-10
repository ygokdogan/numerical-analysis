# Sayısal Analiz | C

**Türkçe** | [English](README.en.md)

**Kök bulma, lineer cebir, türev, integral ve interpolasyon yöntemlerini tek bir terminal uygulamasında birleştiren sayısal analiz projesi.**

Yapısal Programlama dersi kapsamında geliştirilen bu uygulama, kullanıcıdan alınan fonksiyonlar ve matrisler üzerinde sayısal hesaplamalar yapar. Matematiksel ifadeler çalışma anında ayrıştırılır; sonuçlar seçilen yönteme göre kök, hata, iterasyon sayısı, matris veya çözüm vektörü olarak gösterilir.

Harici bir sayısal hesaplama kütüphanesi gerektirmez. Algoritmalar, ifade ayrıştırıcısı ve bellek yönetimi C ile uygulanmıştır.

## Yöntemler

| Menü | Yöntem | İşlev |
| :---: | --- | --- |
| 1 | Bisection | İşaret değiştiren bir aralıkta ikiye bölme ile kök arama |
| 2 | Regula-Falsi | Doğrusal yaklaşım ile kök arama |
| 3 | Newton-Raphson | Başlangıç tahmini ve sayısal türev kullanarak kök arama |
| 4 | Gauss-Jordan | N × N matrisin tersini hesaplama |
| 5 | Cholesky (LU)* | Alt ve üst üçgensel ayrıştırma ile lineer sistem çözme |
| 6 | Gauss-Seidel | Lineer denklem sistemini iteratif olarak çözme |
| 7 | Sayısal türev | İleri, geri ve merkezi fark sonuçlarını birlikte hesaplama |
| 8 | Simpson 1/3 ve 3/8 | İki integral yaklaşımını birlikte hesaplama |
| 9 | Trapez | Belirli integrali trapez yaklaşımıyla hesaplama |
| 10 | Gregory-Newton | Eşit aralıklı noktalardan ileri fark interpolasyonu |

\* Menüde “Cholesky (LU)” olarak adlandırılan uygulama, `U` matrisinin köşegenini 1 alan **Crout tipi LU ayrıştırması** yapar; klasik `A = L Lᵀ` Cholesky ayrıştırması değildir.

## Derleme ve çalıştırma

Gereksinimler: C99 destekleyen GCC veya Clang ve bir terminal. Komutları proje klasöründe çalıştırın.

### Linux / macOS

```bash
cc -std=c99 -O2 numerical_analysis.c -o program -lm
./program
```

### Windows (MinGW-w64 / GCC)

```powershell
gcc -std=c99 -O2 numerical_analysis.c -o program.exe -lm
.\program.exe
```

`-lm`, matematik kütüphanesini bağlar. Uygulama açıldığında yöntem numarasını girin; ardından ekrandaki parametre istemlerini takip edin. Ana menüde `0` seçeneği uygulamayı kapatır.

## Fonksiyon yazımı

Fonksiyon girişinde yalnızca ifadeyi yazın; `f(x) =` eklemeyin.

| Öğe | Desteklenen yazım |
| --- | --- |
| Değişken ve sabit | `x`, `pi` |
| Temel işlemler | `+`, `-`, `*`, `/`, `^` |
| Gruplama | `(` ve `)` |
| Trigonometrik fonksiyonlar | `sin`, `cos`, `tan`, `cot` |
| Ters trigonometrik fonksiyonlar | `arcsin`, `arccos`, `arctan` |
| Üstel ve logaritmik fonksiyonlar | `exp(x)`, `ln(x)`, `log(x)` |
| Tabanlı logaritma | `log_5(x)`; değişken taban için `log_x(8)` |

Örnek ifadeler:

```text
x^3 - 2*x - 5
sin(x) + x^2 - 3
exp(x) - 4
ln(x) + log_2(x)
cos(pi*x)
```

- Çarpma işaretini açıkça yazın: `2*x` ve `2*(x+1)`.
- Ondalık ayırıcı olarak nokta kullanın: `0.001`.
- Fonksiyon adlarını küçük harfle yazın; trigonometrik açılar radyan cinsindedir.
- `ln` doğal logaritma, `log` ise 10 tabanında logaritmadır.
- Kareköklü ifadeler için `x^0.5` kullanılabilir; `sqrt` tanımlı değildir.

## Örnek: √2 değerini bulma

Ana menüden **1 — Bisection Method** seçin ve sırasıyla şu değerleri girin:

| Parametre | Değer |
| --- | --- |
| Fonksiyon | `x^2 - 2` |
| Ondalık basamak sayısı | `6` |
| Alt sınır | `1` |
| Üst sınır | `2` |
| Tolerans | `0.000001` |
| Maksimum iterasyon | `100` |

Çıktı:

```text
--- RESULT ---
  Root      : 1.414214
  Error     : 0.000001
  Iteration : 20
```

İntegral için **8 — Simpson's Methods** seçeneğinde `x^2`, 6 ondalık basamak, `[0, 1]` aralığı ve `n = 6` kullanıldığında her iki yöntem de `0.333333` sonucunu verir.

## Uygulama yapısı

```text
Fonksiyon metni → Token dizisi → Postfix ifade → Yığın ile değerlendirme
                                                    ↓
                                              Sayısal yöntem
                                                    ↓
                                               Terminal çıktısı
```

İfade ayrıştırıcısı, işlem önceliği ve parantezleri dikkate alarak infix gösterimi postfix biçimine dönüştürür. Aynı ifade farklı `x` değerlerinde değerlendirilerek kök bulma, türev ve integral yöntemlerinde kullanılır. Matrisler ve interpolasyon tabloları dinamik bellekle oluşturulur.

## Proje dosyaları

| Dosya / klasör | İçerik |
| --- | --- |
| [`numerical_analysis.c`](numerical_analysis.c) | Algoritmalar, ifade ayrıştırıcısı ve terminal menüsü |
| [`screenshots/`](screenshots/) | Uygulamaya ait ekran görüntüleri |
| [`.gitignore`](.gitignore) | Derleme çıktıları, yerel ayarlar ve arşivler için dışlama kuralları |
| [`README.md`](README.md) | Türkçe dokümantasyon |
| [`README.en.md`](README.en.md) | İngilizce dokümantasyon |

## Kullanım notları

- Bisection ve Regula-Falsi için sürekli bir fonksiyon ve uçlarında zıt işaretli değerler veren bir aralık seçin.
- Newton-Raphson sayısal türev kullanır. Başlangıç tahmini ve sıfırdan farklı türev adımı `h`, yakınsamayı etkiler.
- Gauss-Seidel her sistemde yakınsamaz; köşegeni baskın matrisler uygun bir başlangıçtır. LU uygulaması pivotlama yapmaz.
- Simpson 1/3 için tek girilen `n`, bir artırılarak çift yapılır. Simpson 3/8 uygulaması her aralığı kendi içinde üçe böldüğünden pozitif `n` değerinin 3'ün katı olması gerekmez.
- Gregory-Newton yöntemi eşit aralıklı `x` değerleri gerektirir.
- Ondalık basamak seçimi çıktı biçimini belirler; yakınsama toleransı ayrı bir parametredir.
- Girdi doğrulaması sınırlıdır. Geçerli ifadeler, pozitif boyutlar ve yöntemlere uygun parametreler kullanın; logaritma ve bölme gibi işlemlerin tanım koşullarını gözetin.

Bu proje, sayısal yöntemlerin C ile uygulanmasını incelemek ve örnek problemler üzerinde çalışmak için hazırlanmıştır.
